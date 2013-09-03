/**
 *  \file IMP/misc/SoftCylinderPairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_SOFT_CYLINDER_PAIR_SCORE_H
#define IMPMISC_SOFT_CYLINDER_PAIR_SCORE_H

#include <IMP/misc/misc_config.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base/Pointer.h>
#include <IMP/pair_macros.h>

IMPMISC_BEGIN_NAMESPACE

//! Apply a function to the distance between the cylinders defined by two bonds.
/** The two endpoints of the bond must have the same radius and that is used
    for the radius of the cylinder connecting them.
*/
class IMPMISCEXPORT SoftCylinderPairScore : public PairScore {
  double k_;

 public:
  SoftCylinderPairScore(double k);
  virtual double evaluate_index(kernel::Model *m, const kernel::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(SoftCylinderPairScore);
  IMP_OBJECT_METHODS(SoftCylinderPairScore);
  ;
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_SOFT_CYLINDER_PAIR_SCORE_H */
