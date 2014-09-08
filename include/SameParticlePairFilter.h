/**
 *  \file SameParticlePairFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_SAME_PARTICLE_PAIR_FILTER_H
#define IMPMEMBRANE_SAME_PARTICLE_PAIR_FILTER_H

#include "membrane_config.h"
#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//!
/** Filter particles belonging to the same rigid body
*/
class IMPMEMBRANEEXPORT SameParticlePairFilter : public PairPredicate
{
public:
  SameParticlePairFilter();

  virtual int get_value_index(kernel::Model *m,
  const kernel::ParticleIndexPair &p) const
  IMP_OVERRIDE;

  virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
   const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_PAIR_PREDICATE_METHODS(SameParticlePairFilter);
  IMP_OBJECT_METHODS(SameParticlePairFilter);
};


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_SAME_PARTICLE_PAIR_FILTER_H */
