/**
 *   Copyright 2007-2016 IMP Inventors. All rights reserved
 */
#include <IMP/base_types.h>
#include <IMP/flags.h>

namespace IMP {
void testf();
void testf() {
  Ints its;
  std::cout << its << std::endl;
}
}
namespace IMP {
namespace sub {
void testf();
void testf() {
  Ints its;
  std::cout << its << std::endl;
}
}
}

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test stream IO.");
  using namespace IMP;
  Ints its;
  std::cout << its << std::endl;
  return 0;
}
