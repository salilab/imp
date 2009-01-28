/**
 *  \file CartesianProductSampler.h
 *  \brief The class samples all  combinations in a discrete set
 *         to particles. Each anchor point can be mapped to one
 *         particle for a specific mapping. For example, for anchor points
 *         [1,2,3,4,5] to particles [a,b,c,d] the sampling space would be:
 *         [1,1,1,1],[1,1,1,2],[1,1,1,3],[1,1,1,4],......,[5,5,5,4],[5,5,5,5]
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPDOMINO_CARTESIAN_PRODUCT_SAMPLER_H
#define IMPDOMINO_CARTESIAN_PRODUCT_SAMPLER_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/domino/DiscreteSampler.h"
#include "IMP/base_types.h"
#include "MappedDiscreteSet.h"
#include <algorithm>

IMPDOMINO_BEGIN_NAMESPACE

class  IMPDOMINOEXPORT CartesianProductSampler :  public DiscreteSampler
{
public:
  CartesianProductSampler(){}
  //! Create a combination sampler.
  /**
    \param[in] ds           the discrete sampling space
    \param[in] ps           the sampled particles
  */
  CartesianProductSampler(MappedDiscreteSet *ds, Particles *ps);
  void show(std::ostream& out = std::cout) const;
  void populate_states_of_particles(Particles *particles,
               std::map<std::string, CombState *> *states) const;
  virtual void move2state(const CombState *cs);

protected:
  MappedDiscreteSet *ds_;
  Particles *ps_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_CARTESIAN_PRODUCT_SAMPLER_H */
