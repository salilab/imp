/**
 *  \file ProjectionOverlapFilterTable.cpp
 *  \brief
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/ProjectionOverlapFilterTable.h"

IMPEM2D_BEGIN_NAMESPACE

void ProjectionOverlapFilterTable::do_show(std::ostream &out) const {
  out << "ProjectionOverlapFilterTable" << std::endl;
}

domino::SubsetFilter*
ProjectionOverlapFilterTable::get_subset_filter(const domino::Subset &subset,
                           const domino::Subsets &prior_subsets) const {
   // Check if the subset is the same as the one that this table is filter
   // applies to
  if(subset.size() > 1) {
    return NULL;
  }
  domino::Subset intersection;// = domino::get_intersection(subset, my_subset_);
  if(intersection.size() != 0) {
    domino::SubsetFilter *p = new ProjectionOverlapFilter(image_,
                                                        projection_states_);
    return p;
  }
  return NULL;
}


double ProjectionOverlapFilterTable::get_strength(
                              const domino::Subset  &susbset,
                              const domino::Subsets &prior_subsets) const {
  return 1;
}

IMPEM2D_END_NAMESPACE
