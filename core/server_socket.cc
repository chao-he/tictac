
#include <sys/socket.h>
#include <cstdlib>
#include "core/event_base.h"
#include "core/connection.h"
#include "core/server_socket.h"
#include "core/uv_error_log.h"

class ServerSocket::EventHandlerImpl: public AcceptorEventHandler {
  public:
    EventHandlerImpl(EventBase* base, ServerSocket* parent)
      : AcceptorEventHandler(base)
      , server_(parent) { }

    void Accept() override {
      std::unique_ptr<AsyncIOStream> client(new AsyncIOStream(event_base()->loop()));

      struct sockaddr_in addr;
      int len = sizeof(addr);
      uv_tcp_getpeername(client->transport(), (struct sockaddr*)&addr, &len);
      char sockname[64];
      uv_ip4_name(&addr, sockname, sizeof(sockname));
      fprintf(stderr, "connect from %s:%d\n", sockname, addr.sin_port);

      if (0 == UvAccept(server_->listener_->stream(), client->stream())) {
        UvRead(client->stream());
        new Connection(server_->event_base(), std::move(client));
      } else {
        fprintf(stderr, "Accept fail\n");
      }
      // auto conn = ConnectionManager::NewConnection(std::move(client));
    }
  private:
    ServerSocket* server_;
};

ServerSocket::ServerSocket(EventBase* base)
  : base_(base)
  , listener_(new AsyncIOStream(base->loop()))
  , handler_(new EventHandlerImpl(base, this)) {
    listener_->stream()->data = handler_.get();
  }

ServerSocket::~ServerSocket() {
  DetachEventBase();
}

void ServerSocket::AttachEventBase(EventBase* base) {
  base_ = base;
}

void ServerSocket::DetachEventBase() {
  base_ = NULL;
}

void ServerSocket::Bind(int port) {
  Bind("0.0.0.0", port);
}

void ServerSocket::Bind(const char* ip, int port) {
  UvBind(listener_->transport(), ip, port);
  listener_->reuse_addr();
  listener_->reuse_port();
}

void ServerSocket::Listen(int backlog) {
  UvListen(listener_->stream(), backlog);
}
