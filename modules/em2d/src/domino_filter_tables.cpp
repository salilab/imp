/**
 *  \file domino_filter_tables.cpp
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/domino_filter_tables.h"
#include "IMP/em2d/domino_filters.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE


void DistanceFilterTable::do_show(std::ostream &out) const {
  this->show(out);
}


domino::SubsetFilter* DistanceFilterTable::get_subset_filter(
                            const domino::Subset &subset,
                            const domino::Subsets &prior_subsets) const {
  IMP_UNUSED(prior_subsets.size());
  IMP_LOG_VERBOSE( " get_subset_filter " << std::endl);
  subset.show();
  // The subset must contain the particles of my_subset_
  for(domino::Subset::const_iterator it = my_subset_.begin();
                                     it != my_subset_.end();
                                     ++it) {
    if (! std::binary_search(subset.begin(), subset.end(), *it)) {
      return nullptr;
    }
  }
  // If the particles are in any of the prior subsets, a filter for them has
  // been created already. Do not create it again
  for(domino::Subsets::const_iterator it = prior_subsets.begin();
      it != prior_subsets.end();
      ++it ) {
    if( std::includes(it->begin(), it->end(),
      my_subset_.begin(), my_subset_.end()) ) {
      return nullptr;
    }
  }
  domino::SubsetFilter *p = new DistanceFilter(subset,
                                               my_subset_,
                                               ps_table_,
                                               max_distance_);
  return p;
}

double DistanceFilterTable::get_strength(
                              const domino::Subset  &subset,
                              const domino::Subsets &prior_subsets) const {
  IMP_UNUSED(subset);
  IMP_UNUSED(prior_subsets.size());
  return 1;
}


IMPEM2D_END_NAMESPACE
