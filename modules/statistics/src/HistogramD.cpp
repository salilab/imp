/**
 *  \file Histogram.cpp  \brief simple histogram class
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/statistics/HistogramD.h>
#include <numeric>
IMPSTATISTICS_BEGIN_NAMESPACE
double get_quantile(const Histogram1D &h, double fraction) {
  IMP_USAGE_CHECK(fraction >= 0. && fraction <= 1,
                  "The input number is not a fraction\n");
  double stop_count = h.get_total_count() * fraction;
  int partial_count = 0;
  for (unsigned int i = 0; i < h.get_counts().get_number_of_voxels(); i++) {
    algebra::GridIndexD<1> gi(Ints(1, i));
    partial_count += h.get_counts()[i];
    if (partial_count > stop_count) {
      return h.get_counts().get_center(gi)[0];
    }
  }
  return h.get_counts().get_bounding_box().get_corner(1)[0];
}
IMPSTATISTICS_END_NAMESPACE
