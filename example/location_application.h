#pragma once

#include "core/application.h"

class LocationServiceApplication: public Application {
  public:
    LocationServiceApplication();
    int Run() override;
};
