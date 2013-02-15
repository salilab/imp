/**
 *  \file domino_filters.h
 *  \brief SubsetFilter for checking overlap between projections and images
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DOMINO_FILTERS_H
#define IMPEM2D_DOMINO_FILTERS_H

#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/domino_particle_states.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/domino/subset_filters.h"
#include "IMP/domino/Assignment.h"
#include "IMP/Pointer.h"
#include "IMP/base_types.h"
#include "IMP/log.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

class IMPEM2DEXPORT DistanceFilter: public domino::SubsetFilter {

protected:
  domino::Subset subset_acting_on_;
  domino::Subset subset_restrained_;
  Pointer<domino::ParticleStatesTable> ps_table_;
  double max_distance_;

public:

  /*! Filter action on the distance between two particles. To be used with
      DOMINO
    \param[in] subset_to_act_on A subset that is going to be filtered.
    \param[in] subset_to_restrain A subset of 2 particles that need to be
               within a given distance
    \param[in] ps_table table containing the states for subset_to_act_on and
               subset_to_restraint
    \param[in] max_distance The maximum distance between the particles in
               subset_to_restrain
  */
  DistanceFilter(const domino::Subset &subset_to_act_on,
                 const domino::Subset &subset_to_restrain,
                 domino::ParticleStatesTable *ps_table,
                 double max_distance) : subset_acting_on_(subset_to_act_on),
                                        subset_restrained_(subset_to_restrain),
                                        ps_table_(ps_table),
                                          max_distance_(max_distance) {
    IMP_LOG_TERSE( "DistanceFilter created" << std::endl);
  }
  void show(std::ostream &out = std::cout) const {
    out << "DistanceFilter" << std::endl;
  }
  IMP_SUBSET_FILTER(DistanceFilter);
};

IMP_OBJECTS(DistanceFilter,DistanceFilters);

IMPEM2D_END_NAMESPACE


#endif  /* IMPEM2D_DOMINO_FILTERS_H */
