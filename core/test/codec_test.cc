#include "location.h"
#include "codec.h"
#include <iostream>

struct Foo {
  const std::string& name() const { return name_; }
  void set_name(const std::string& n) { name_ = n; }
  std::string name_;
};
int main() {
  PbKVCodec<Foo> codec(&Foo::name); 
  Foo foo;
  foo.set_name("xxxxx");
  std::cout << codec.Encode(foo).second << "\n";
  return 0;
}
