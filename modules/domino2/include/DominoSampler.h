/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_DOMINO_SAMPLER_H
#define IMPDOMINO2_DOMINO_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "DiscreteSampler.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE



//! Sample best solutions using Domino
/**
 */
class IMPDOMINO2EXPORT DominoSampler : public DiscreteSampler
{
  Pointer<SubsetStatesTable> sst_;
 public:
  DominoSampler(Model *m);
  DominoSampler(Model*m, ParticleStatesTable *pst);
  IMP_DISCRETE_SAMPLER(DominoSampler);
};


IMP_OBJECTS(DominoSampler, DominoSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_DOMINO_SAMPLER_H */
