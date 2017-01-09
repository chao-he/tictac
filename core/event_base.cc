
#include <cstdlib>
#include "core/event_base.h"

EventBase::EventBase() {
  uv_loop_init(&loop_);
}

EventBase::~EventBase() {
  // uv_loop_close(loop_);
  // uv_loop_delete(loop_);
  // free(loop_);
  // loop_ = NULL;
  uv_loop_close(&loop_);
}

void EventBase::Run() {
  uv_run(&loop_, UV_RUN_DEFAULT);
}

void EventBase::Stop() {
  uv_stop(&loop_);
}
