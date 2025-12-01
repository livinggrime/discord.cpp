#include <discord/gateway/websocket_client.h>
#include <discord/utils/logger.h>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <zlib.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace discord {

using websocket_client = websocketpp::client<websocketpp::config::asio_tls_client>;

class WebSocketClient::Impl {
public:
    Impl() : is_connected_(false), compression_enabled_(false), 
              zlib_stream_(), reconnect_manager_(std::make_unique<ReconnectionManager>()) {
        client_.set_access_channels(websocketpp::log::alevel::none);
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.set_error_channels(websocketpp::log::elevel::none);
        
        client_.init_asio();
        
        client_.set_tls_init_handler([](websocketpp::connection_hdl) {
            auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
            ctx->set_options(boost::asio::ssl::context::default_workarounds);
            return ctx;
        });
        
        client_.set_open_handler([this](websocketpp::connection_hdl hdl) {
            is_connected_ = true;
            connection_hdl_ = hdl;
            reconnect_manager_->handle_connection_restored();
            LOG_INFO("WebSocket connection established");
        });
        
        client_.set_close_handler([this](websocketpp::connection_hdl hdl) {
            is_connected_ = false;
            auto con = client_.get_con_from_hdl(hdl);
            auto close_code = con->get_remote_close_reason();
            auto reason = con->get_remote_close_reason();
            
            LOG_WARN("WebSocket connection closed: " + std::to_string(close_code) + " " + reason);
            
            if (close_callback_) {
                close_callback_(close_code, reason);
            }
            
            // Handle reconnection
            reconnect_manager_->handle_disconnect(close_code, reason);
        });
        
        client_.set_message_handler([this](websocketpp::connection_hdl hdl, websocket_client::message_ptr msg) {
            try {
                std::string payload_str;
                
                // Check if message is compressed (binary messages)
                if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
                    payload_str = decompress_message(msg->get_payload());
                } else {
                    payload_str = msg->get_payload();
                }
                
                auto payload = nlohmann::json::parse(payload_str);
                
                // Handle gateway events that affect reconnection
                if (payload.contains("op")) {
                    int opcode = payload["op"];
                    if (opcode == static_cast<int>(GatewayOpcode::HELLO)) {
                        // Start heartbeat
                        if (payload.contains("d") && payload["d"].contains("heartbeat_interval")) {
                            int interval = payload["d"]["heartbeat_interval"];
                            start_heartbeat(interval);
                        }
                    } else if (opcode == static_cast<int>(GatewayOpcode::INVALID_SESSION)) {
                        bool can_resume = payload.value("d", false);
                        reconnect_manager_->handle_invalid_session(can_resume);
                    } else if (opcode == static_cast<int>(GatewayOpcode::RECONNECT)) {
                        reconnect_manager_->handle_disconnect(1000, "Reconnect requested by Discord");
                    }
                }
                
                if (event_callback_) {
                    event_callback_(payload);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to parse WebSocket message: " + std::string(e.what()));
            }
        });
        
        // Setup reconnection callbacks
        reconnect_manager_->set_callbacks(
            [this](bool should_resume) {
                if (should_resume) {
                    resume();
                } else {
                    identify();
                }
            },
            [this]() {
                resume();
            }
        );
    }
    
    bool connect(const std::string& url) {
        try {
            websocketpp::lib::error_code ec;
            websocket_client::connection_ptr con = client_.get_connection(url, ec);
            
            if (ec) {
                return false;
            }
            
            client_.connect(con);
            
            thread_ = std::thread([this]() {
                try {
                    client_.run();
                } catch (const std::exception& e) {
                    is_connected_ = false;
                }
            });
            
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    void disconnect() {
        if (is_connected_) {
            client_.close(connection_hdl_, websocketpp::close::status::normal, "");
        }
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    
    bool is_connected() const {
        return is_connected_;
    }
    
    void send(const nlohmann::json& payload) {
        if (is_connected_) {
            try {
                client_.send(connection_hdl_, payload.dump(), websocketpp::frame::opcode::text);
            } catch (const std::exception& e) {
                // Silent error handling
            }
        }
    }
    
    void on_event(EventCallback callback) {
        event_callback_ = std::move(callback);
    }
    
    void on_close(CloseCallback callback) {
        close_callback_ = std::move(callback);
    }
    
    void set_token(const std::string& token) {
        token_ = token;
    }
    
    void set_intents(int intents) {
        intents_ = intents;
    }
    
    void identify() {
        nlohmann::json identify;
        identify["op"] = 2;
        identify["d"] = nlohmann::json{
            {"token", token_},
            {"intents", intents_},
            {"properties", nlohmann::json{
                {"os", "linux"},
                {"browser", "discord.cpp"},
                {"device", "discord.cpp"}
            }}
        };
        send(identify);
    }
    
    void resume() {
        if (session_id_.empty()) {
            identify();
            return;
        }
        
        nlohmann::json resume;
        resume["op"] = 6;
        resume["d"] = nlohmann::json{
            {"token", token_},
            {"session_id", session_id_},
            {"seq", last_sequence_}
        };
        send(resume);
        LOG_INFO("Attempting to resume session");
    }
    
    void start_heartbeat(int interval_ms) {
        if (heartbeat_thread_.joinable()) {
            heartbeat_thread_.join();
        }
        
        heartbeat_thread_ = std::thread([this, interval_ms]() {
            while (is_connected_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                
                if (is_connected_) {
                    nlohmann::json heartbeat;
                    heartbeat["op"] = 1;
                    heartbeat["d"] = last_sequence_;
                    send(heartbeat);
                }
            }
        });
    }
    
    void enable_auto_reconnect(bool enabled) {
        reconnect_manager_->enable_auto_reconnect(enabled);
    }
    
    void set_reconnection_config(int max_retries, std::chrono::milliseconds base_delay, std::chrono::milliseconds max_delay) {
        reconnect_manager_->set_max_retries(max_retries);
        reconnect_manager_->set_base_delay(base_delay);
        reconnect_manager_->set_max_delay(max_delay);
    }
    
    bool is_reconnecting() const {
        return reconnect_manager_->is_reconnecting();
    }
    
    void stop_reconnecting() {
        reconnect_manager_->stop_reconnecting();
    }
    
    void enable_compression(bool enabled) {
        compression_enabled_ = enabled;
        if (enabled) {
            initialize_compression();
        } else {
            cleanup_compression();
        }
    }
    
    void initialize_compression() {
        compression_buffer_.resize(COMPRESSION_BUFFER_SIZE);
        
        zlib_stream_.zalloc = Z_NULL;
        zlib_stream_.zfree = Z_NULL;
        zlib_stream_.opaque = Z_NULL;
        zlib_stream_.avail_in = 0;
        zlib_stream_.next_in = Z_NULL;
        
        if (inflateInit2(&zlib_stream_, 15 + 32) != Z_OK) {
            LOG_ERROR("Failed to initialize zlib inflation");
            compression_enabled_ = false;
        }
    }
    
    void cleanup_compression() {
        if (compression_enabled_) {
            inflateEnd(&zlib_stream_);
        }
    }
    
    std::string decompress_message(const std::string& compressed_data) {
        if (!compression_enabled_) {
            return compressed_data;
        }
        
        // Discord sends zlib-compressed messages with 4-byte header
        if (compressed_data.size() < 4) {
            return compressed_data;
        }
        
        // Skip the 4-byte header
        const char* data_ptr = compressed_data.data() + 4;
        size_t data_size = compressed_data.size() - 4;
        
        zlib_stream_.avail_in = static_cast<uInt>(data_size);
        zlib_stream_.next_in = reinterpret_cast<const Bytef*>(data_ptr);
        
        std::string result;
        do {
            zlib_stream_.avail_out = static_cast<uInt>(compression_buffer_.size());
            zlib_stream_.next_out = compression_buffer_.data();
            
            int ret = inflate(&zlib_stream_, Z_NO_FLUSH);
            
            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                LOG_ERROR("Zlib decompression error: " + std::to_string(ret));
                return compressed_data; // Return original data on error
            }
            
            size_t have = compression_buffer_.size() - zlib_stream_.avail_out;
            result.append(reinterpret_cast<char*>(compression_buffer_.data()), have);
            
        } while (zlib_stream_.avail_out == 0);
        
        return result;
    }

private:
    websocket_client client_;
    std::thread thread_;
    std::thread heartbeat_thread_;
    websocketpp::connection_hdl connection_hdl_;
    std::atomic<bool> is_connected_;
    std::string token_;
    int intents_;
    std::string session_id_;
    int last_sequence_;
    bool compression_enabled_;
    
    // Compression support
    z_stream zlib_stream_;
    std::vector<uint8_t> compression_buffer_;
    static constexpr size_t COMPRESSION_BUFFER_SIZE = 8192;
    
    EventCallback event_callback_;
    CloseCallback close_callback_;
    std::unique_ptr<ReconnectionManager> reconnect_manager_;
};

WebSocketClient::WebSocketClient() : pImpl(std::make_unique<Impl>()) {}

WebSocketClient::~WebSocketClient() = default;

bool WebSocketClient::connect(const std::string& url) {
    return pImpl->connect(url);
}

void WebSocketClient::disconnect() {
    pImpl->disconnect();
}

bool WebSocketClient::is_connected() const {
    return pImpl->is_connected();
}

void WebSocketClient::send(const nlohmann::json& payload) {
    pImpl->send(payload);
}

void WebSocketClient::on_event(EventCallback callback) {
    pImpl->on_event(std::move(callback));
}

void WebSocketClient::on_close(CloseCallback callback) {
    pImpl->on_close(std::move(callback));
}

void WebSocketClient::set_token(const std::string& token) {
    pImpl->set_token(token);
}

void WebSocketClient::set_intents(int intents) {
    pImpl->set_intents(intents);
}

void WebSocketClient::identify() {
    pImpl->identify();
}

void WebSocketClient::enable_auto_reconnect(bool enabled) {
    pImpl->enable_auto_reconnect(enabled);
}

void WebSocketClient::set_reconnection_config(int max_retries, 
                                           std::chrono::milliseconds base_delay,
                                           std::chrono::milliseconds max_delay) {
    pImpl->set_reconnection_config(max_retries, base_delay, max_delay);
}

bool WebSocketClient::is_reconnecting() const {
    return pImpl->is_reconnecting();
}

void WebSocketClient::stop_reconnecting() {
    pImpl->stop_reconnecting();
}

void WebSocketClient::enable_compression(bool enabled) {
    pImpl->enable_compression(enabled);
}

bool WebSocketClient::is_compression_enabled() const {
    return pImpl->compression_enabled_;
}

} // namespace discord