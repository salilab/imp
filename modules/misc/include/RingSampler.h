/**
 *  \file RingSampler.h
 *  \brief sample configurations of a ring
 *
 */

#ifndef IMPMISC_RING_SAMPLER_H
#define IMPMISC_RING_SAMPLER_H

#include "config.h"

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/domino/TransformationPermutationSampler.h"
#include <IMP/domino/DiscreteSet.h>
#include <IMP/domino/TransformationPermutationSampler.h>
#include "IMP/base_types.h"
#include "AnchorPointSubset.h"
#include <algorithm>
#include  "combination.hpp"

IMPMISC_BEGIN_NAMESPACE
//! Sample configurations of a ring
class IMPMISCEXPORT RingSampler :
      public IMP::domino::TransformationPermutationSampler
{
public:
  //! Costructor
  /**
  */
  RingSampler(){}
  RingSampler(domino::DiscreteSet *ds, Particles *ps):
    domino::TransformationPermutationSampler(ds,ps,true){}
  ~RingSampler(){};

  protected:
};
IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_RING_SAMPLER_H */
