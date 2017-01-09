
#include <assert.h>
#include "core/network.h"
#include "core/event_handler.h"
#include "core/uv_error_log.h"

IPAddressV4::IPAddressV4(uint32_t src)
  : addr_() {
    addr_.inAddr_.s_addr = src;
  }

IPAddressV4::IPAddressV4(const char* addr)
  : addr_() {
    if (inet_pton(AF_INET, addr, &addr_.inAddr_) == -1) {
      addr_.inAddr_.s_addr = 0;
    }
  }

template <
class IntegralType,
      IntegralType DigitCount,
      IntegralType Base = 10,
      bool PrintAllDigits = false,
      class = typename std::enable_if<
      std::is_integral<IntegralType>::value &&
      std::is_unsigned<IntegralType>::value,
      bool>::type>
      inline void writeIntegerString(IntegralType val, char** buffer) {
        char* buf = *buffer;

        if (!PrintAllDigits && val == 0) {
          *(buf++) = '0';
          *buffer = buf;
          return;
        }

        IntegralType powerToPrint = 1;
        for (int i = 1; i < DigitCount; ++i) {
          powerToPrint *= Base;
        }

        bool found = PrintAllDigits;
        while (powerToPrint) {
          if (found || powerToPrint <= val) {
            IntegralType value = val / powerToPrint;
            if (Base == 10 || value < 10) {
              value += '0';
            } else {
              value += ('a' - 10);
            }
            *(buf++) = value;
            val %= powerToPrint;
            found = true;
          }

          powerToPrint /= Base;
        }

        *buffer = buf;
      }

inline std::string fastIpv4ToString(const in_addr& inAddr) {
  const uint8_t* octets = reinterpret_cast<const uint8_t*>(&inAddr.s_addr);
  char str[sizeof("255.255.255.255")];
  char* buf = str;

  writeIntegerString<uint8_t, 3>(octets[0], &buf);
  *(buf++) = '.';
  writeIntegerString<uint8_t, 3>(octets[1], &buf);
  *(buf++) = '.';
  writeIntegerString<uint8_t, 3>(octets[2], &buf);
  *(buf++) = '.';
  writeIntegerString<uint8_t, 3>(octets[3], &buf);

  return std::string(str, buf - str);
}

std::string IPAddressV4::str() const {
  return fastIpv4ToString(addr_.inAddr_);
}

SocketAddress::SocketAddress(const char* ip, int port)
  : port_(port) {
    strncpy(ip_, ip, sizeof(ip_));
    uv_ip4_addr(ip_, port_, &addr_);
  }

template<typename Arg1, typename Arg2, typename Arg3>
static void uv_callback(EventType event, Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  auto handler = reinterpret_cast<EventHandler*>(arg1->data);
  EventHandler::EventData data;
  data.arg1 = reinterpret_cast<uintptr_t>(arg1);
  data.arg2 = arg2;
  data.arg3 = reinterpret_cast<uintptr_t>(arg3);
  handler->HandleEvent(event, data);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  uv_callback(kAlloc, handle, suggested_size, buf);
}

static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  uv_callback(kReadComplete, stream, nread, buf);
}

static void write_cb(uv_write_t* req, int status) {
  uv_callback(kWriteComplete, req->handle, status, req);
}

static void connection_cb(uv_stream_t* server, int status) {
  // uv_tcp_t client;
  // uv_tcp_init(server->loop, &client);
  // fprintf(stderr, "accepted_fd = %d, listen_fd = %d, %d\n", server->accepted_fd,  server->io_watcher.fd, client.io_watcher.fd);
  // int r = uv_accept(server, (uv_stream_t*)&client);
  // UV_ERROR_LOG(r);
  uv_callback(kAccepted, server, status, nullptr);
}

static void shutdown_cb(uv_shutdown_t* req, int status) {
  uv_callback(kShutdown, req->handle, status, req);
}

static void close_cb(uv_handle_t* handle) {
  uv_callback(kClosed, handle, 0, nullptr);
}

static void connect_cb(uv_connect_t* req, int status) {
  uv_callback(kConnected, req->handle, status, req);
}

int UvListen(uv_stream_t* self, int backlog) {
  int r = uv_listen(self, backlog, connection_cb);
  UV_ERROR_LOG(r);
  return r;
}

int UvBind(uv_tcp_t* self, const struct sockaddr* addr, unsigned int flags) {
  int r = uv_tcp_bind(self, addr, flags);
  UV_ERROR_LOG(r);
  return r;
}

int UvBind(uv_tcp_t* self, const char* ip, int port) {
  // SocketAddress addr(ip, port);
  struct sockaddr_in addr;
  uv_ip4_addr(ip, port, &addr);
  int r = uv_tcp_bind(self, (struct sockaddr*)&addr, 0);
  UV_ERROR_LOG(r);
  return r;
}

int UvAccept(uv_stream_t* server, uv_stream_t* client) {
  int r = uv_accept(server, client);
  UV_ERROR_LOG(r);
  return r;
}

int UvRead(uv_stream_t* self) {
  int r = uv_read_start(self, alloc_cb, read_cb);
  UV_ERROR_LOG(r);
  return r;
}

int UvWrite(uv_stream_t* self, uv_write_t* req, uv_buf_t buf) {
  int r = uv_write(req, self, &buf, 1, write_cb);
  UV_ERROR_LOG(r);
  return r;
}

int UvClose(uv_stream_t* self) {
  uv_close((uv_handle_t*)self, close_cb);
  return 0;
}

int UvShutdown(uv_stream_t* self, uv_shutdown_t* req) {
  int r = uv_shutdown(req, self, shutdown_cb);
  UV_ERROR_LOG(r);
  return r;
}

int UvConnect(uv_tcp_t* self, uv_connect_t* req, const char*ip, int port) {
  struct sockaddr_in addr;
  uv_ip4_addr(ip, port, &addr);
  int r = uv_tcp_connect(req, (uv_tcp_t*)self, (const struct sockaddr*)&addr, connect_cb);
  UV_ERROR_LOG(r);
  return r;
}
