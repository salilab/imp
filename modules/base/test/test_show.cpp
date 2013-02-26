/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/base/Showable.h>
#include <IMP/base/VersionInfo.h>
#include <IMP/base/check_macros.h>
namespace IMP {
  namespace base {
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
}
int main(int, char *[]) {
  IMP_CATCH_AND_TERMINATE(  IMP::base::test(); );
  return 0;
}
