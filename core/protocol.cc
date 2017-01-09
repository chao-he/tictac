
#include <vector>
#include <cstdint>
#include <cstdio>
#include "core/redis_protocol.h"

Protocol* Protocol::NewProtocol() {
  return new RedisProtocol;
}
