/**
 *  \file SameParticlePairFilter.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_SAME_PARTICLE_PAIR_FILTER_H
#define IMPSPB_SAME_PARTICLE_PAIR_FILTER_H

#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//!
/** Filter particles belonging to the same rigid body
*/
class IMPSPBEXPORT SameParticlePairFilter : public PairPredicate {
 public:
  SameParticlePairFilter();

  virtual int get_value_index(
      IMP::Model *m, const IMP::ParticleIndexPair &p) const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_PAIR_PREDICATE_METHODS(SameParticlePairFilter);
  IMP_OBJECT_METHODS(SameParticlePairFilter);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SAME_PARTICLE_PAIR_FILTER_H */
