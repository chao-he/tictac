#include <iostream>
#include "file.h"

int main() {
  for (auto &line : fs::file_iter("test")) {
    std::cout<<line;
  }
}
