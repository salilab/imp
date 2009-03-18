/**
 *  \file PermutationSampler.h
 *  \brief The class samples all  permutations of transformations to particles
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#ifndef IMPDOMINO_TRANSFORMATION_PERMUTATION_SAMPLER_H
#define IMPDOMINO_TRANSFORMATION_PERMUTATION_SAMPLER_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/domino/PermutationSampler.h"
#include "IMP/domino/TransformationMappedDiscreteSet.h"
#include "IMP/base_types.h"
#include "DiscreteSet.h"
#include <algorithm>
#include "IMP/domino/TransformationUtils.h"
IMPDOMINO_BEGIN_NAMESPACE

//! Sample all permutations of transformations on particles
/**
 */
class IMPDOMINOEXPORT TransformationPermutationSampler:public PermutationSampler
{
public:
  TransformationPermutationSampler(){}
  //! Create a permutation sampler.
  /**
    \param[in] ds           the discrete sampling space
    \param[in] ps           the sampled particles
  */
  TransformationPermutationSampler(DiscreteSet *ds,
                                   Particles *ps,bool trans_from_orig);
  void move2state(const CombState *cs);
protected:
  TransformationUtils tu_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_TRANSFORMATION_PERMUTATION_SAMPLER_H */
