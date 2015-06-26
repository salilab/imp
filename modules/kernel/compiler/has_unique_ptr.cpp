/**
 *  \file nothing.cpp
 *  \brief nothing
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <memory>
int main(void) {
  std::unique_ptr<int> a(new int);
  return 0;
}
