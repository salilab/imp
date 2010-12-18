/**
 *  \file RepulsiveDistancePairScore.h
 *  \brief A simple quadric repulsive term between two atoms. Restraint is zero
 *  when the distance equals the sum of the radii plus the shift.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H
#define IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H

#include "isd_config.h"
#include <IMP/core/XYZR.h>
#include <IMP/PairScore.h>
#include <IMP/Pointer.h> 

IMPISD_BEGIN_NAMESPACE

//! A repulsive potential on the distance between two atoms
/** \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see LennardJonesPairScore
 */
class IMPISDEXPORT RepulsiveDistancePairScore : public PairScore
{
  double x0_, k_;
public:
  
  RepulsiveDistancePairScore(double d0, double k);

  double get_rest_length() const {
    return x0_;
  }
  double get_stiffness() const {
    return k_;
  }

  
  IMP_SIMPLE_PAIR_SCORE(RepulsiveDistancePairScore);
};


IMP_OBJECTS(RepulsiveDistancePairScore,RepulsiveDistancePairScores);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H */
