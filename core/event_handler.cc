#include "core/uv_error_log.h"
#include "core/event_handler.h"

void AcceptorEventHandler::HandleEvent(EventType event, EventData edata) {
  int status = edata.arg2;
  UV_ERROR_LOG(status);
  switch(event) {
    case kAccepted:
      Accept();
      break;
    default:
      break;
  }
}

void TransportEventHandler::HandleEvent(EventType event, EventData edata) {
  switch(event) {
    case kAlloc:
      Alloc(edata.arg2, reinterpret_cast<uv_buf_t*>(edata.arg3));
      break;
    case kShutdown:
      Shutdown(reinterpret_cast<uv_shutdown_t*>(edata.arg3));
      break;
    case kClosed:
      Closed();
      break;
    case kReadComplete:
      ReadComplete(reinterpret_cast<uv_buf_t*>(edata.arg3), edata.arg2);
      break;
    case kWriteComplete:
      WriteComplete(reinterpret_cast<uv_write_t*>(edata.arg3), edata.arg2);
      break;
    default:
      break;
  }
}
