#pragma once

#include <memory>
#include "core/network.h"
#include "core/event_handler.h"

class ServerSocket: public SocketBase {
  public:
    explicit ServerSocket(EventBase* evb);
    virtual ~ServerSocket();

  public:
    EventBase* event_base() const override {
      return base_;
    }

    void AttachEventBase(EventBase*);
    void DetachEventBase();

    void Bind(int port);
    void Bind(const char* ip, int port);
    void Listen(int backlog);
    
    uv_stream_t* stream() { return listener_->stream(); }
    uv_tcp_t* transport() { return listener_->transport(); }

  public:
    class EventHandlerImpl;

  protected:
    EventBase*        base_;
    std::unique_ptr<AsyncIOStream>    listener_;
    std::unique_ptr<EventHandlerImpl> handler_;
};
