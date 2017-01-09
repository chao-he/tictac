#pragma once

#include "core/network.h"

class Request {
  public:
    virtual ~Request() {}
};

class RequestHandler {
  public:
    virtual ~RequestHandler() { }
    virtual void HandleRequest(Request* req, Connection* conn) = 0;
    static RequestHandler* NewRequestHandler();
};

