/**
 *  \file PermutationSampler.h
 *  \brief The class samples all  permutations in a discrete set
 *         to particles. Each anchor point can be mapped to one
 *         particle for a specific mapping. For example, for anchor points
 *         [1,2,3,4,5,6] to particles [a,b,c,d] the sampling space would be:
 *         [1,2,3,4],[1,2,3,5],[1,2,3,6],[1,2,4,3],......,[6,5,4,2],[6,5,4,3]
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPDOMINO_PERMUTATION_SAMPLER_H
#define IMPDOMINO_PERMUTATION_SAMPLER_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/domino/DiscreteSampler.h"
#include "IMP/base_types.h"
#include "DiscreteSet.h"
#include <algorithm>
#include  "combination.hpp"

IMPDOMINO_BEGIN_NAMESPACE

//! Permutation sampler
/**
 */
class  IMPDOMINOEXPORT PermutationSampler :  public DiscreteSampler
{
public:
  PermutationSampler(){}
  //! Create a permutation sampler.
  /**
    \param[in] ds           the discrete sampling space
    \param[in] ps           the sampled particles
  */
  PermutationSampler(DiscreteSet *ds, Particles *ps);
  void show(std::ostream& out = std::cout) const;
  void populate_states_of_particles(Particles *particles,
               std::map<std::string, CombState *> *states) const;
  void move2state(const CombState *cs);

protected:
  //  void construct();
  void clear();
  DiscreteSet *ds_;
  Particles *ps_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_PERMUTATION_SAMPLER_H */
