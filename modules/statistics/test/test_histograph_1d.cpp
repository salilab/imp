/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/HistogramD.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test HistogramD.");

  IMP::statistics::HistogramD<1> h(1.0, IMP::algebra::BoundingBox1D(0, 10));
  h.add(1.0);
  return 0;
}
