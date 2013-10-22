/**
 *  \file IMP/isd/RepulsiveDistancePairScore.h
 *  \brief A simple quadric repulsive term between two atoms. Restraint is zero
 *  when the distance equals the sum of the radii plus the shift.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H
#define IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H

#include <IMP/isd/isd_config.h>
#include <IMP/core/XYZR.h>
#include <IMP/PairScore.h>
#include <IMP/base/Pointer.h>
#include <IMP/generic.h>

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

  virtual double evaluate_index(kernel::Model *m,
                                const kernel::ParticleIndexPair &p,
                                DerivativeAccumulator *da)
      const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
                                         const kernel::ParticleIndexes &pis)
      const IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(RepulsiveDistancePairScore);
  IMP_OBJECT_METHODS(RepulsiveDistancePairScore);;
};


IMP_OBJECTS(RepulsiveDistancePairScore,RepulsiveDistancePairScores);

IMPISD_END_NAMESPACE

#endif  /* IMPISD_REPULSIVE_DISTANCE_PAIR_SCORE_H */
