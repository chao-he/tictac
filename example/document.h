#pragma once
#include "userprofile.pb.h"

class Document {
  public:
    UserProfile user;
    bool ParseFromString(const std::string& str) {
      return user.ParseFromString(str);
    }
};
