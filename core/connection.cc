#include "core/connection.h"
#include "core/request.h"
#include "core/protocol.h"
#include "core/uv_error_log.h"


class Connection::EventHandlerImpl: public TransportEventHandler {
  public:
    EventHandlerImpl(EventBase* base, Connection* conn)
      : TransportEventHandler(base)
      , parent_(conn) { }

    void Alloc(int suggested_size, uv_buf_t* buf) override {
      parent_->recv_buffer_.expand(suggested_size);
      buf->base = parent_->recv_buffer_.read_buf();
      buf->len = suggested_size;
      // fprintf(stderr, "alloc read buffer = %p + %d\n", buf->base, buf->len);
    }

    void ReadComplete(uv_buf_t* buf, int nread) override {
      if (nread > 0) {
        parent_->recv_buffer_.advance_write(nread);
        parent_->protocol_->Consume(parent_, buf->base, nread);
        // UvRead(parent_->stream_->stream());
      } else if (nread == 0) {
        fprintf(stderr, "client closed the connection\n");
        parent_->Close();
      } else {
        parent_->Shutdown();
        UV_ERROR_LOG(nread);
      }
    }

    void WriteComplete(uv_write_t* req, int status) override {
      if (req != NULL) {
        if (req->nbufs > sizeof(req->bufsml) / sizeof(req->bufsml[0])) {
          for (size_t i = 0; i < req->nbufs; ++ i) {
            // fprintf(stderr, "%lu - %p\n", i, req->bufs[i].base);
            if (req->bufs[i].base) {
              free(req->bufs[i].base);
              req->bufs[i].base = NULL;
            }
          }
        } else {
          for (size_t i = 0; i < req->nbufs; ++ i) {
            // fprintf(stderr, "%lu - %p\n", i, req->bufsml[i].base);
            if (req->bufsml[i].base) {
              free(req->bufsml[i].base);
              req->bufsml[i].base = NULL;
            }
          }
        }
        delete req;
      }
      UV_ERROR_LOG(status);
    }

    void Shutdown(uv_shutdown_t* req) override {
      UvClose(parent_->stream_->stream());
      delete req;
    }

    void Closed() override {
      delete parent_;
    }

  private:
    Connection* parent_;
};

Connection::Connection(EventBase* base, std::unique_ptr<AsyncIOStream> client)
  : base_(base)
  , stream_(std::move(client))
  , protocol_(Protocol::NewProtocol())
  , request_handler_(RequestHandler::NewRequestHandler())
  , event_handler_(new EventHandlerImpl(base, this)) {
    stream_->stream()->data = event_handler_.get();
}

Connection::Connection(EventBase* base, std::unique_ptr<AsyncIOStream> client,
    std::unique_ptr<Protocol> protocol,
    std::unique_ptr<RequestHandler> request_handler)
    : base_(base)
    , stream_(std::move(client))
    , protocol_(std::move(protocol))
    , request_handler_(std::move(request_handler))
    , event_handler_(new EventHandlerImpl(base, this)) {
      stream_->stream()->data = event_handler_.get();
    }

Connection::~Connection() {}

void Connection::AsyncWrite(char* data, size_t len) {
  uv_write_t* req = new uv_write_t;
  memset(req, sizeof(req), 0);
  req->data = event_handler_.get();
  uv_buf_t buf;
  buf.base = data;
  buf.len = len;
  UvWrite(stream_->stream(), req, buf);
  fprintf(stderr, "alloc %p\n", data);
}

void Connection::AsyncWrite(const char* data, size_t len) {
  char* copy = (char*)malloc(len);
  memmove(copy, data, len);
  AsyncWrite(copy, len);
}

void Connection::Shutdown() {
  uv_shutdown_t* req = new uv_shutdown_t;
  req->data = event_handler_.get();
  UvShutdown(stream_->stream(), req);
}

void Connection::Close() {
  UvClose(stream_->stream());
}

void Connection::Process(Request* req) {
  request_handler_->HandleRequest(req, this);
}

// void WriteRequest::Add(char* data, size_t len) {
//   uv_buf_t buf = uv_buf_init(data, len);
//   iov.emplace_back(buf);
// }

// void WriteRequest::Add(const char* data, size_t len) {
//   uv_buf_t buf = uv_buf_init((char*)data, len);
//   iov.emplace_back(buf);
//   need_free = false;
// }

// WriteRequest::~WriteRequest() {
//   if (need_free) {
//     for (auto& buf: iov) {
//       if (buf.base) {
//         ::free(buf.base);
//         buf.base = NULL;
//         buf.len = 0;
//       }
//     }
//   }
// }

// WriteRequest::WriteRequest(Connection* owner)
//   : conn(owner)
//   , status(-1)
//   , need_free(true) {
//     req.data = this;
//   }
