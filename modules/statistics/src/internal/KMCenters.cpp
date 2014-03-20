/**
 * \file KMCenters.cpp
 * \brief Holds the centers of the clusters
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/KMCenters.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
KMCenters::KMCenters(int k, KMData* p) : data_points_(p) {
  centers_ = allocate_points(k, data_points_->get_dim());
}
KMCenters::~KMCenters() { deallocate_points(centers_); }
void KMCenters::show(std::ostream& out) const {
  print_points("Center_Points", *centers_, out);
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE
