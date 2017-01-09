#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <array>
#include <string>
#include <uv.h>

class EventBase;
class EventHandler;
class Connection;
class Protocol;
class Request;
class RequestHandler;

struct Counter {
  long nr_read_bytes = 0;
  long nr_requests = 0;

  void incr(int bytes) {
    nr_read_bytes += bytes;
    ++ nr_requests;
  }
};

typedef std::array<uint8_t, 4> ByteArray4;

class IPAddressV4 {
  public:

    IPAddressV4()
      : addr_() {
      }

    explicit IPAddressV4(const in_addr ipaddr)
      : addr_(ipaddr) {
      }

    explicit IPAddressV4(uint32_t addr);
    explicit IPAddressV4(const char* addr);

    std::string str() const;
    uint32_t toLong() const { return addr_.inAddr_.s_addr; }

  private:
    union AddressStorage {
      static_assert(sizeof(in_addr) == sizeof(ByteArray4),
          "size of in_addr and ByteArray4 are different");
      in_addr inAddr_;
      ByteArray4 bytes_;
      AddressStorage() {
        std::memset(this, 0, sizeof(AddressStorage));
      }
      explicit AddressStorage(const ByteArray4 bytes): bytes_(bytes) {}
      explicit AddressStorage(const in_addr addr): inAddr_(addr) {}
    } addr_;
};

class SocketAddress {
  public:
    SocketAddress(const char* host, int port);
    explicit SocketAddress(struct sockaddr_in* addr);

    const char* ip() { return ip_; }
    int port() { return port_; }
    struct sockaddr* get_sockaddr() { return (struct sockaddr*)&addr_; }
  private:
    char ip_[16];
    int port_;
    struct sockaddr_in addr_;
};

class SocketBase {
  public:
    virtual EventBase* event_base() const = 0;
    virtual ~SocketBase() = default;
};

class AsyncIOStream {
  public:
    AsyncIOStream() = default;
    AsyncIOStream(uv_loop_t* loop) {
      attach(loop);
    }

    void attach(uv_loop_t* loop) { uv_tcp_init(loop, &stream_); }
    uv_tcp_t* transport() { return &stream_; }
    uv_stream_t* stream() { return (uv_stream_t*)&stream_; }
    int fd() const { return stream_.io_watcher.fd; }  

    void reuse_port() {
      int opt = 1;
      setsockopt(fd(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    }

    void reuse_addr() {
      int opt = 1;
      setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
  private:
    uv_tcp_t stream_;
};

int UvListen(uv_stream_t* self, int backlog);
int UvAccept(uv_stream_t* self, uv_stream_t* client);
int UvRead(uv_stream_t* self);
int UvWrite(uv_stream_t* self, uv_write_t* req, uv_buf_t buf);
int UvClose(uv_stream_t* self);
int UvShutdown(uv_stream_t* self, uv_shutdown_t* req);
int UvBind(uv_tcp_t* self, const struct sockaddr* addr, unsigned int flags);
int UvBind(uv_tcp_t* self, const char* ip, int port);
int UvConnect(uv_tcp_t* self, uv_connect_t* req, const char*ip, int port);
