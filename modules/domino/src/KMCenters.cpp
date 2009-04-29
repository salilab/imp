/**
 * \file KMCenters.cpp
 * \brief Holds the centers of the clusters
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/domino/KMCenters.h>
IMPDOMINO_BEGIN_NAMESPACE
KMCenters::KMCenters(int k, KMData *p)
  : data_points_(p) {
  centers_ = allocate_points(k,data_points_->get_dim());
}
KMCenters::~KMCenters() {
  deallocate_points(centers_);
}
void KMCenters::show(std::ostream& out) const{
  print_points("Center_Points", *centers_,out);
}
IMPDOMINO_END_NAMESPACE
