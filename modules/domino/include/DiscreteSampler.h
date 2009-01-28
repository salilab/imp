/**
 *  \file DiscreteSampler.h   \brief Storage of a discrete sampling space
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_DISCRETE_SAMPLER_H
#define IMPDOMINO_DISCRETE_SAMPLER_H

#include "config.h"

#include <IMP/Particle.h>
#include "CombState.h"
#include <vector>

IMPDOMINO_BEGIN_NAMESPACE

//! Holds the states of a single or a set of particles
//! This interface is used by the DOMINO optimizer to get states of particles.
/** \note Since this class is header-only, we must not mark it for
          export with IMPDOMINOEXPORT.
 */
class  DiscreteSampler
{
public:
  virtual ~DiscreteSampler(){};

  virtual void show(std::ostream& out = std::cout) const {}

  //! Show the sampling space of a single particle
  virtual void show_space(Particle *p,
                      std::ostream& out = std::cout) const {}

  //! Fill states as encoded in the node for the input subset of particles
  /** \param[in] particles   a set of particles for which combinations
                             of states should be generated.
      \param[in] states      the dataset to be filled with states.
   */
  virtual void populate_states_of_particles(Particles *particles,
              std::map<std::string, CombState *> *states) const{}

  //! Set the attributes of the particles in the combination to the states
  //! indicated in the combination
  virtual void move2state(const CombState *cs) {}
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DISCRETE_SAMPLER_H */
