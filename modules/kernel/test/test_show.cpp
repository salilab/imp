/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Showable.h>
#include <IMP/VersionInfo.h>
#include <IMP/check_macros.h>
#include <IMP/flags.h>

namespace IMP {
namespace {
void test() {
  std::cout << Showable(1);
  std::cout << Showable("hi");
  std::cout << Showable(VersionInfo("hi", "there"));
  std::cout << VersionInfo("hi", "there");
  std::cout << Vector<int>(10);
  std::cout << Showable(Vector<int>(10));
}
}
}

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test showable.");
  IMP_CATCH_AND_TERMINATE(IMP::test(););
  return 0;
}
