#pragma once

#include "core/network.h"

class Protocol { 
  public:
    virtual ~Protocol() {}
    virtual void Consume(Connection* conn, char* data, int len) = 0;
    static Protocol* NewProtocol();
};
