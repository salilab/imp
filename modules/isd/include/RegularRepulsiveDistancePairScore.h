/**
 *  \file RegularRepulsiveDistancePairScore.h
 *  \brief A simple quadric repulsive term between two atoms. Restraint is zero
 *  when the distance equals the sum of the radii plus the shift.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_REGULAR_REPULSIVE_DISTANCE_PAIR_SCORE_H
#define IMPISD_REGULAR_REPULSIVE_DISTANCE_PAIR_SCORE_H

#include "isd_config.h"
#include <IMP/core/XYZR.h>
#include <IMP/PairScore.h>
#include <IMP/Pointer.h> 
#include <IMP/atom/LennardJones.h>
#include <IMP/atom/smoothing_functions.h>


IMPISD_BEGIN_NAMESPACE

//! A regular repulsive potential on the distance between two atoms
/** \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see LennardJonesPairScore
 */
class IMPISDEXPORT RegularRepulsiveDistancePairScore : public PairScore
{
  double e_;
  IMP::internal::OwnerPointer<atom::SmoothingFunction> smoothing_function_;


public: 
  RegularRepulsiveDistancePairScore(double e, atom::SmoothingFunction *f): e_(e), smoothing_function_(f) {};
  

  double get_amplitude() const { return e_;}

  
  IMP_SIMPLE_PAIR_SCORE(RegularRepulsiveDistancePairScore);
};


IMP_OBJECTS(RegularRepulsiveDistancePairScore,RegularRepulsiveDistancePairScores);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_REGULAR_REPULSIVE_DISTANCE_PAIR_SCORE_H */
