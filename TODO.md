# Discord.cpp TODOs

This file tracks all remaining tasks and features to make Discord.cpp a fully-featured Discord API wrapper.  
TODOs are organized by **priority** and **module**, with file references to help implementation.

---

## **Critical Features (High Priority)**

### **Core Module**
- [ ] Complete missing implementations: `Logger`, `CacheManager`, and other empty header files  
  `src/utils/logger.h/cpp`, `src/cache/cache_manager.h/cpp`
- [ ] Fix code duplication in `client.cpp`  
  `src/core/client.cpp`
- [ ] Implement proper error recovery and reconnection logic with exponential backoff  
  `src/core/client.cpp`, `src/gateway/websocket_client.cpp`

### **API Module**
- [ ] Add comprehensive rate limiting with bucket management and global limits  
  `src/api/rate_limiter.h/cpp`

### **Gateway Module**
- [ ] Implement Shard Manager with auto-sharding and load balancing  
  `src/gateway/shard_manager.h/cpp`
- [ ] Add Gateway compression and zlib-stream support for better performance  
  `src/gateway/websocket_client.h/cpp`

### **Events Module**
- [ ] Implement comprehensive event system with collectors and filters  
  `src/events/event_dispatcher.h/cpp`, `src/events/event_handlers.h/cpp`

### **Components / Interactions**
- [ ] Implement interactive Components system  
  `src/api/rest_endpoints.h/cpp`, `src/events/middleware.h/cpp`
  - Buttons (all styles, disabled, emojis)
  - Select Menus (String, User, Role, Channel, Mentionable)
  - Modals (pop-up forms with text inputs)
  - Action Rows (layout containers)
  - Components V2 (Sections, Text Display)

### **Commands**
- [ ] Complete Slash Commands framework  
  `src/commands/command_manager.h/cpp`
  - Command builders
  - Registration system (global & guild)
  - Autocomplete
  - Permission system
  - Localization
- [ ] Implement Context Menus  
  `src/commands/command_manager.h/cpp`
  - User context
  - Message context
  - Command context

### **Voice Support**
- [ ] Voice connections (join/leave channels)  
  `src/gateway/websocket_client.h/cpp`
- [ ] Audio streaming (Opus encoding/decoding)  
  `src/gateway/voice_client.h/cpp` (placeholder)
- [ ] Voice encryption (XSalsa20-Poly1305)  
  `src/gateway/voice_client.h/cpp` (placeholder)
- [ ] Voice state updates  
  `src/gateway/websocket_client.h/cpp`

---

## **Important Features (Medium Priority)**

### **Guild & Channels**
- [ ] Implement Threads API (create/manage/archive threads, thread members)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Add Polls support (create/manage/vote/end polls)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Stage Channels (create/manage stage instances)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Forum Channels (create/manage forum posts)  
  `src/api/rest_endpoints.h/cpp`

### **Events & Moderation**
- [ ] Implement Guild Scheduled Events (create/manage/RSVP)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Add AutoModeration (rules, actions, execution)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Implement Permission resolution (channel overrides, role hierarchy)  
  `src/core/client.h/cpp`, `src/api/rest_endpoints.h/cpp`
- [ ] Add Audit Log access & filtering  
  `src/api/rest_endpoints.h/cpp`

### **Caching & Performance**
- [ ] Implement advanced caching strategies (Redis clustering, invalidation)  
  `src/cache/redis_cache.h/cpp`
- [ ] Add event collectors and filters  
  `src/events/event_dispatcher.h/cpp`

### **Content Management**
- [ ] Implement Webhook management (create/modify/execute)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Add Emoji & Sticker management  
  `src/api/rest_endpoints.h/cpp`
- [ ] Embed builders (rich embeds)  
  `src/utils/embed_builder.h/cpp`

---

## **Advanced Features (Lower Priority)**

### **Monetization & Enterprise**
- [ ] Add Monetization APIs (subscriptions, one-time purchases, entitlements)  
  `src/api/rest_endpoints.h/cpp`
- [ ] Implement Application Commands Permissions v2  
  `src/commands/command_manager.h/cpp`

### **Community & Onboarding**
- [ ] Implement Welcome Screen and onboarding flow  
  `src/api/rest_endpoints.h/cpp`
- [ ] Guild templates (create/apply from template)  
  `src/api/rest_endpoints.h/cpp`

### **Testing & Documentation**
- [ ] Implement comprehensive testing suite (unit tests, integration tests, mocks)  
  `tests/` folder
- [ ] Add extensive documentation and usage examples for all features  
  `docs/` folder

---

**Usage Notes:**  
- Start with **Critical Features**, then move to **Important**, then **Advanced**.  
- Each task includes **module/file references** to make navigation easier.  
- Mark `[x]` when a task is completed.  
- Update this file regularly to track progress and plan incremental development.
