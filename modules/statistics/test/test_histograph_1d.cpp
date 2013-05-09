/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/HistogramD.h>

int main(int, char * []) {
  IMP::statistics::HistogramD<1> h(1.0, IMP::algebra::BoundingBox1D(0, 10));
  h.add(1.0);
  return 0;
}
