/**
 *  \file domino2/BranchAndBoundSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H
#define IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "DiscreteSampler.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE



//! Sample best solutions using BranchAndBound
/** Find all good configurations of the model using branch and bound.
    Searches are truncated when the score is worse than the the thresholds
    in the Sampler or when two particles with the same ParticlesState
    are assigned the same state.
 */
class IMPDOMINO2EXPORT BranchAndBoundSampler : public DiscreteSampler
{
public:
  BranchAndBoundSampler(Model *m);
  BranchAndBoundSampler(Model*m, ParticleStatesTable *pst);
  IMP_SAMPLER(BranchAndBoundSampler);
};


IMP_OBJECTS(BranchAndBoundSampler, BranchAndBoundSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H */
