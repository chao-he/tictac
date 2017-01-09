#pragma once

#include <uv.h>

class EventBase {
  public:
    EventBase();
    //
    // explicit EventBase(uv_loop_t* loop)
    //   : loop_(loop) {
    //   }

    virtual ~EventBase();
    void Run();
    void Stop();
   
    uv_loop_t* loop() { return &loop_; }

  private:
    uv_loop_t loop_;
};
