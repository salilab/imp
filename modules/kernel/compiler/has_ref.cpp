/**
 *  \file nothing.cpp
 *  \brief nothing
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <functional>

bool testfunc(char c) { return false; }

int main(void) {
  std::reference_wrapper<bool(char)> x = std::ref(testfunc);
  return 0;
}
