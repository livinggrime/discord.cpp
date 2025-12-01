# Architecture Diagrams

## Module Dependency Graph

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
                      ┌──────────────┐
                      │  discord.h   │  ← Single entry point
                      └──────┬───────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
    ┌────────┐           ┌────────┐          ┌────────┐
    │ core.h │           │ api.h  │          │events.h│
    └────────┘           └────────┘          └────────┘
        │                    │                    │
        ├─────────┬──────────┤                    │
        │         │          │                    │
        ▼         ▼          ▼                    ▼
    ┌─────────────────────────────────────────────────┐
    │         Gateway Module (gateway.h)              │
    │  ┌──────────────────────────────────────────┐   │
    │  │ - WebSocket Connection                   │   │
    │  │ - Gateway Events Handling                │   │
    │  │ - Automatic Reconnection                 │   │
    │  │ - Shard Management                       │   │
    │  └──────────────────────────────────────────┘   │
    └──────────────┬──────────────────────────────────┘
                   │
    ┌──────────────┴──────────────┐
    │                             │
    ▼                             ▼
┌─────────────┐            ┌────────────┐
│ cache.h     │            │ commands.h │
└─────────────┘            └────────────┘
    │
    ▼
┌─────────────────────────────────────────┐
│        Utilities Module (utils.h)       │
│  ┌─────────────────────────────────┐   │
│  │ - Types & Result<T>             │   │
│  │ - Logging                       │   │
│  │ - Thread Pool                   │   │
│  │ - Configuration                 │   │
│  │ - Authentication                │   │
│  │ - Embed Building                │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

## Module Interaction Flow

```
┌───────────────────────────────────┐
│    User Application Code           │
└───────────────┬─────────────────────┘
                │
                ▼
        ┌─────────────────┐
        │ DiscordClient   │
        │   (core)        │
        └────────┬────────┘
                 │
        ┌────────┴────────┐
        │                 │
        ▼                 ▼
    ┌─────────┐      ┌──────────────┐
    │  HTTP   │      │  WebSocket   │
    │ Client  │      │  Client      │
    │ (api)   │      │ (gateway)    │
    └────┬────┘      └──────┬───────┘
         │                  │
         │   ┌──────────────┴──────────┐
         │   │                         │
         ▼   ▼                         ▼
    ┌────────────┐         ┌──────────────────┐
    │ Rate       │         │ Gateway          │
    │ Limiter    │         │ Events           │
    │ (api)      │         │ (gateway)        │
    └────────────┘         └────────┬─────────┘
                                    │
                                    ▼
                           ┌──────────────────┐
                           │ Event            │
                           │ Dispatcher       │
                           │ (events)         │
                           └────────┬─────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    │               │               │
                    ▼               ▼               ▼
              ┌─────────┐    ┌────────────┐    ┌───────────┐
              │ Command │    │ Event      │    │ Custom    │
              │ Manager │    │ Handlers   │    │ Handlers  │
              │(commands)    │(events)    │    │(user)     │
              └─────────┘    └────────────┘    └───────────┘
                    │               │               │
                    └───────────────┴───────────────┘
                                    │
                                    ▼
                            ┌─────────────────┐
                            │ Cache           │
                            │ Manager         │
                            │ (cache)         │
                            └─────────────────┘
```

## Data Flow Diagram

```
User Creates Bot
        │
        ▼
  Initialize DiscordClient
        │
        ├─ Create HTTPClient (api)
        ├─ Create WebSocketClient (gateway)
        ├─ Create EventDispatcher (events)
        ├─ Create CacheManager (cache)
        └─ Create CommandManager (commands)
        │
        ▼
  Register Event Handlers
  (on_ready, on_message, etc.)
        │
        ▼
  Call client.run()
        │
        ├──────────────────────────┐
        │                          │
        ▼                          ▼
  HTTPClient                WebSocketClient
  (REST API calls)          (Real-time connection)
        │                          │
        ▼                          ▼
  Discord API              Discord Gateway
  (Responses)              (Event Stream)
        │                          │
        ▼                          ▼
  Parse JSON              Parse Gateway Events
        │                          │
        ▼                          ▼
  Update Cache            Dispatch Events
        │                          │
        ▼                          ▼
  Return Results          Call Event Handlers
                                   │
                    ┌──────────────┼──────────────┐
                    │              │              │
                    ▼              ▼              ▼
              Process &      Log &           Prepare
              Respond        Track           Response
                    │              │              │
                    └──────────────┴──────────────┘
                                   │
                                   ▼
                          Send Response to Discord
                          (via HTTPClient/WebSocket)
```

## Request/Response Cycle

```
┌──────────────────────────────────────────────────────────────────┐
│ User Code: client.send_message(channel_id, "Hello")               │
└─────────────────────────┬──────────────────────────────────────────┘
                          │
                          ▼
                  ┌───────────────────┐
                  │ DiscordClient     │
                  │ (core module)     │
                  └────────┬──────────┘
                           │
                           ▼
                  ┌───────────────────┐
                  │ Build Request     │
                  │ - URL endpoint    │
                  │ - Headers         │
                  │ - Body (message)  │
                  └────────┬──────────┘
                           │
                           ▼
                  ┌───────────────────────────┐
                  │ Check Rate Limiter (api)  │
                  └────────┬──────────────────┘
                           │
                   ┌───────┴────────┐
                   │                │
          (Wait)   │ (Ready)        │
                   │                │
                   ▼                │
              ┌─────────┐            │
              │ Queue   │            │
              │ Request │            │
              └────┬────┘            │
                   │                 │
                   └────────┬────────┘
                            │
                            ▼
                  ┌─────────────────────┐
                  │ HTTPClient Worker   │
                  │ (api)               │
                  │ - Setup CURL        │
                  │ - Execute Request   │
                  │ - Parse Response    │
                  └────────┬────────────┘
                           │
                           ▼
                  ┌──────────────────────┐
                  │ Network              │
                  │ (HTTPS to Discord)   │
                  └─────────┬────────────┘
                            │
                            ▼
                  ┌──────────────────────┐
                  │ Discord API Server   │
                  │ - Process Request    │
                  │ - Send Response      │
                  └─────────┬────────────┘
                            │
                            ▼
                  ┌──────────────────────┐
                  │ Network              │
                  │ (Response from API)  │
                  └─────────┬────────────┘
                            │
                            ▼
                  ┌──────────────────────┐
                  │ HTTPClient           │
                  │ - Receive Response   │
                  │ - Parse JSON         │
                  └────────┬─────────────┘
                           │
                           ▼
                  ┌──────────────────────┐
                  │ Cache Manager        │
                  │ - Store Message      │
                  │ - Update Channel     │
                  └────────┬─────────────┘
                           │
                           ▼
                  ┌──────────────────────┐
                  │ Return Result        │
                  │ (to user code)       │
                  └──────────────────────┘
```

## Module Internal Structure

```
┌─ discord/ (namespace)
│
├─ core/ (namespace)
│  ├── client.h           → Main DiscordClient class
│  ├── exceptions.h       → Exception hierarchy
│  └── interfaces.h       → Interface contracts
│
├─ api/ (namespace)
│  ├── http_client.h      → CURL-based HTTP client
│  ├── rest_endpoints.h   → Discord REST API definitions
│  └── rate_limiter.h     → Rate limit management
│
├─ gateway/ (namespace)
│  ├── websocket_client.h → WebSocketPP-based connection
│  ├── gateway_events.h   → Event type definitions
│  ├── reconnection.h     → Auto-reconnection logic
│  └── shard_manager.h    → Multi-shard support
│
├─ events/ (namespace)
│  ├── event_dispatcher.h → Event system core
│  ├── event_handlers.h   → Handler registration
│  └── middleware.h       → Event middleware
│
├─ cache/ (namespace)
│  ├── cache_manager.h    → Cache abstraction
│  ├── memory_cache.h     → In-memory cache impl
│  └── redis_cache.h      → Redis-backed cache
│
├─ utils/ (namespace)
│  ├── types.h            → Result<T>, common types
│  ├── logger.h           → Logging utilities
│  ├── thread_pool.h      → Thread pool
│  ├── config_manager.h   → Configuration
│  ├── auth.h             → Authentication
│  └── embed_builder.h    → Embed building
│
└─ commands/ (namespace)
   └── command_manager.h  → Command registration/dispatch
```

## Class Relationships

```
┌────────────────────────────────────────────────────────┐
│                    DiscordClient                       │
│  (Orchestrator - ties all modules together)           │
│                                                        │
│  - has: HTTPClient                                     │
│  - has: WebSocketClient                                │
│  - has: EventDispatcher                                │
│  - has: CacheManager                                   │
│  - has: CommandManager                                 │
│  - has: Logger                                         │
│  - has: ThreadPool                                     │
└────────────────────────────────────────────────────────┘
                            ▲
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼
┌─────────────┐    ┌──────────────┐   ┌──────────────┐
│ HTTPClient  │    │ WebSocket    │   │ Event        │
│             │    │ Client       │   │ Dispatcher   │
│ interface:  │    │              │   │              │
│ - get()     │    │ interface:   │   │ interface:   │
│ - post()    │    │ - connect()  │   │ - on()       │
│ - put()     │    │ - send()     │   │ - emit()     │
│ - patch()   │    │              │   │              │
│ - delete()  │    │              │   │              │
└──────┬──────┘    └──────┬───────┘   └──────┬───────┘
       │                  │                  │
       ▼                  ▼                  ▼
┌─────────────────┐  ┌──────────────┐  ┌──────────────┐
│ RateLimiter     │  │ Reconnection │  │ Middleware   │
│                 │  │ Manager      │  │              │
│ - can_request() │  │              │  │ - process()  │
│ - wait_for_slot()  │ - reconnect()│  │              │
└─────────────────┘  └──────────────┘  └──────────────┘
```

## Dependency Injection Points

```
┌─────────────────────────────────────────────────────┐
│           DiscordClient Constructor                 │
└────────────┬────────────────────────────────────────┘
             │
             ├─ Token (required)
             │
             ├─ Optional Dependencies:
             │  ├─ ICache implementation
             │  │  └─ MemoryCache (default)
             │  │  └─ RedisCache (alternative)
             │  │
             │  ├─ IHttpClient implementation
             │  │  └─ HTTPClient (built-in)
             │  │
             │  ├─ IWebSocketClient implementation
             │  │  └─ WebSocketClient (built-in)
             │  │
             │  ├─ ILogger implementation
             │  │  └─ Logger (built-in)
             │  │
             │  ├─ IThreadPool implementation
             │  │  └─ ThreadPool (built-in)
             │  │
             │  └─ IRateLimiter implementation
             │     └─ RateLimiter (built-in)
             │
             └─ Configuration Options
                ├─ API version
                ├─ Gateway intents
                ├─ Shard configuration
                ├─ Cache settings
                └─ Logging level
```

This architecture ensures:
- **Modularity**: Each module is independent
- **Flexibility**: Easy to swap implementations
- **Scalability**: Add features without breaking existing code
- **Testability**: Mock interfaces for unit testing
- **Maintainability**: Clear responsibility boundaries
