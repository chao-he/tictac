#pragma once

#include <memory.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <memory>
#include "core/network.h"
#include "core/event_base.h"
#include "core/event_handler.h"
#include "core/socket_buffer.h"

class Connection: public SocketBase {
  public:
    Connection(EventBase* base, std::unique_ptr<AsyncIOStream>);
    Connection(EventBase* base, std::unique_ptr<AsyncIOStream>,
        std::unique_ptr<Protocol>, std::unique_ptr<RequestHandler>);
    ~Connection();

    EventBase* event_base() const override { return base_; }

    void AsyncWrite(char* data, size_t len);
    void AsyncWrite(const char* data, size_t len);
    void Shutdown();
    void Close();
    void Process(Request* req);
  public:
    class EventHandlerImpl;
  
  protected:
    char* AllocReadBuffer(size_t size);
    
  private:
    EventBase* base_;
    SocketBuffer<char> recv_buffer_;
    std::unique_ptr<AsyncIOStream> stream_;
    std::unique_ptr<Protocol> protocol_;
    std::unique_ptr<RequestHandler> request_handler_;
    std::unique_ptr<EventHandlerImpl> event_handler_;
};

// class WriteRequest {
//   public:
//     WriteRequest(Connection* owner);
//     ~WriteRequest();
//
//     void Add(char* data, size_t len);
//     void Add(const char* data, size_t len);
//   private:
//     friend class Connection;
//     Connection* conn{NULL};
//     uv_write_t req;
//     std::vector<uv_buf_t> iov;
//     int status{-1};
//     bool need_free{true};
// };

// struct TemporaryBuffer {
//   TemporaryBuffer()
//     : buffer(64*1024, 0)
//       , used(0) {
//       }
//
//   char* Alloc(size_t size) {
//     auto p = &buffer[0] + used;
//     used += size;
//     return p;
//   }
//
//   void Free(char* buf, size_t len) {
//     (void) len;
//     auto base = &buffer[0];
//     used = buf - base;
//   }
//
//   std::vector<char> buffer;
//   size_t used;
// };

// class ConnectionManager {
//   public:
//     static std::shared_ptr<Connection> NewConnection(std::unique_ptr<AsyncIOStream> client) {
//       auto conn = std::make_shared<Connection>(client);
//       all_connections_[client->stream()->io_watcher.fd] = conn;
//       return conn;
//     }
//   private:
//     static std::vector<std::shared_ptr<Connection>> all_connections_;
// };
