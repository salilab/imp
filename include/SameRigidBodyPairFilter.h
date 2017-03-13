/**
 *  \file SameRigidBodyPairFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_SAME_RIGID_BODY_PAIR_FILTER_H
#define IMPMEMBRANE_SAME_RIGID_BODY_PAIR_FILTER_H

#include "membrane_config.h"
#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//!
/** Filter particles belonging to the same rigid body
*/
class IMPMEMBRANEEXPORT SameRigidBodyPairFilter : public PairPredicate
{
public:
  SameRigidBodyPairFilter();

  virtual int get_value_index(IMP::Model *m,
  const IMP::ParticleIndexPair &p) const
  IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
  IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

 IMP_PAIR_PREDICATE_METHODS(SameRigidBodyPairFilter);
  IMP_OBJECT_METHODS(SameRigidBodyPairFilter);
};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SAME_RIGID_BODY_PAIR_FILTER_H */
