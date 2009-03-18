/**
 *  \file TransformationPermutationSampler.h
 *  \brief The class samples all combinations of a discrete set to particles.
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/domino/TransformationPermutationSampler.h"
IMPDOMINO_BEGIN_NAMESPACE

TransformationPermutationSampler::TransformationPermutationSampler(
        DiscreteSet *ds,Particles *ps, bool trans_from_orig)
        :PermutationSampler(ds,ps),tu_(ps,trans_from_orig)
{}
void TransformationPermutationSampler::move2state(const CombState *cs) {
  Particle *p;
  for (std::map<Particle *,unsigned int>::const_iterator
        it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
    p = it->first;
    tu_.move2state(p,ds_->get_state(it->second));
  }
}

IMPDOMINO_END_NAMESPACE
