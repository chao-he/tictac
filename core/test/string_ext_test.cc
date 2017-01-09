
#include "string_ext.h"
#include <iostream>


int main(int argc, char** argv) {

  std::string line(argv[1]);
  for(auto const& col : ext::Split(line, ",; \t")) {
    std::cout << col << "\n";
  }

}

