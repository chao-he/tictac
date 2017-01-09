
#include <memory>
#include "location_application.h"

int main(int argc, char** argv) {
  std::unique_ptr<Application> app(new LocationServiceApplication());
  // std::unique_ptr<Application> app(new SimpleServer());
  app->Init(argc, argv);
  return app->Run();
}
