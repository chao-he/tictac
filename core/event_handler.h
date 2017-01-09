#pragma once
#include "core/event_base.h"

enum EventType: uint8_t {
  kAccepted = 1,
  kConnected,
  kReadComplete,
  kReadError,
  kWriteComplete,
  kWriteError,
  kShutdown,
  kClosed,
  kAlloc
};

class EventHandler {
  public:
    struct EventData {
      uintptr_t arg1;
      int arg2;
      uintptr_t arg3;
      EventData()
        : arg1(0)
          , arg2(-1)
          , arg3(0) {
          }
    };

    explicit EventHandler(EventBase* base): base_(base) { }
    virtual ~EventHandler() { }
    virtual void HandleEvent(EventType event, EventData edata) = 0;
    EventBase* event_base() { return base_; }
   
  private:
    EventBase* base_;
};

class AcceptorEventHandler: public EventHandler {
  public:
    AcceptorEventHandler(EventBase* base)
      : EventHandler(base) {
      }
    virtual void HandleEvent(EventType event, EventData edata) override;
    virtual void Accept() = 0;;
};

class TransportEventHandler: public EventHandler {
  public:
    TransportEventHandler(EventBase* base)
      : EventHandler(base) {
        }

    virtual void HandleEvent(EventType event, EventData edata) override;
    virtual void Alloc(int suggested_size, uv_buf_t* buf) = 0;
    virtual void ReadComplete(uv_buf_t* buf, int bytes) = 0;
    virtual void WriteComplete(uv_write_t*, int bytes) = 0;
    virtual void Shutdown(uv_shutdown_t*) = 0;
    virtual void Closed() = 0;
};
