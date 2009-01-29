/**
 *  \file TransformationCartesianProductSampler.h
 *  \brief Samples all combinations of a transformations on particles.
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include "IMP/domino/TransformationCartesianProductSampler.h"
IMPDOMINO_BEGIN_NAMESPACE

TransformationCartesianProductSampler::TransformationCartesianProductSampler(
   TransformationMappedDiscreteSet *ds,Particles *ps,
   bool trans_from_orig):CartesianProductSampler(ds,ps),tu_(ps,trans_from_orig)
{
}
void TransformationCartesianProductSampler
        ::move2state(const CombState *cs) {
  Particle *p;
  for (std::map<Particle *,unsigned int>::const_iterator
       it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    tu_.move2state(p,ds_->get_mapped_state(p,it->second));
  }
}

IMPDOMINO_END_NAMESPACE
