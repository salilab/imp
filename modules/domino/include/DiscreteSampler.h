/**
 *  \file DiscreteSampler.h   \brief Storage of a discrete sampling space
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_DISCRETE_SAMPLER_H
#define IMPDOMINO_DISCRETE_SAMPLER_H

#include "domino_config.h"

#include <IMP/Particle.h>
#include "CombState.h"
#include <vector>
#include "DiscreteSet.h"
#include <IMP/container/ListSingletonContainer.h>
IMPDOMINO_BEGIN_NAMESPACE

//! Holds the states of a single or a set of particles
//! This interface is used by the DOMINO optimizer to get states of particles.
/** \note Since this class is header-only, we must not mark it for
          export with IMPDOMINOEXPORT. Note, that this means that you
          cannot derive from it outside of the domino module. Be ware.
 */

class  DiscreteSampler
{
public:
  virtual ~DiscreteSampler(){};

  virtual void show(std::ostream& out = std::cout) const=0;

  //! Show the sampling space of a single particle
  virtual void show_space(Particle */*p*/,
                          std::ostream& /*out*/ = std::cout) const{};

  //! Fill states as encoded in the node for the input subset of particles
  /** \param[in] particles   a set of particles for which combinations
                             of states should be generated.
      \param[in] states      the dataset to be filled with states.
   */
  virtual void populate_states_of_particles(
              container::ListSingletonContainer *particles,
              Combinations *states) const=0;

  IMP_NO_SWIG(
  //! Get the sampling space of a single particle
              virtual DiscreteSet* get_space(Particle *p) const=0;
             )
  //! Set the attributes of the particles in the combination to the states
  //! indicated in the combination
  virtual void move2state(const CombState *cs)=0;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DISCRETE_SAMPLER_H */
