/**
 *   Copyright 2007-2011 IMP Inventors. All rights reserved
 */
#include <IMP/base_types.h>
namespace IMP {
  void testf() {
    Ints its;
    std::cout << its << std::endl;
  }
}
namespace IMP {
  namespace sub {
    void testf() {
      Ints its;
      std::cout << its << std::endl;
    }
  }
}
int main(int, char *[]) {
  using namespace IMP;
  Ints its;
  std::cout << its << std::endl;
  return 0;
}
