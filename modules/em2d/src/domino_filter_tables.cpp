/**
 *  \file domino_filter_tables.cpp
 *  \brief
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_filter_tables.h"
#include "IMP/em2d/domino_filters.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE

/*
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

*/
void DistanceFilterTable::do_show(std::ostream &out) const {
  this->show(out);
}

domino::SubsetFilter* DistanceFilterTable::get_subset_filter(
                            const domino::Subset &subset,
                            const domino::Subsets &prior_subsets) const {
  IMP_LOG(IMP::VERBOSE, " get_subset_filter " << std::endl);
  subset.show();

  // Check that the subset only has 2 particles
  if(subset.size() != 2) return NULL;

  // Check if the subset contains the particles of my_subset
  for(domino::Subset::const_iterator it = subset.begin();
                                     it != subset.end();
                                     ++it) {
    if (! std::binary_search(my_subset_.begin(), my_subset_.end(), *it)) {
      return NULL;
    }
  }

  IMP_LOG(IMP::TERSE, "creating DistanceFilter" << std::endl);
  domino::SubsetFilter *p = new DistanceFilter(my_subset_,
                                               ps_table_,
                                               max_distance_);
  return p;
}

double DistanceFilterTable::get_strength(
                              const domino::Subset  &susbset,
                              const domino::Subsets &prior_subsets) const {
  return 1;
}


IMPEM2D_END_NAMESPACE
