/**
 *  \file DiscreteSampler.h   \brief Storage of a discrete sampling space
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_DISCRETE_SAMPLER_H
#define IMPDOMINO2_DISCRETE_SAMPLER_H

#include "../domino2_config.h"

#include <IMP/Particle.h>
#include "CombState.h"
#include <vector>
#include <IMP/container/ListSingletonContainer.h>
#include "../particle_states.h"
#include "../subset_states.h"
IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

//! Holds the states of a single or a set of particles
//! This interface is used by the DOMINO optimizer to get states of particles.
/** \note Since this class is header-only, we must not mark it for
          export with IMPDOMINO2EXPORT. Note, that this means that you
          cannot derive from it outside of the domino module. Be ware.
 */

class  DiscreteSampler
{
  Pointer<ParticleStatesTable> pst_;
  Pointer<SubsetStatesTable> sst_;
public:
  DiscreteSampler(ParticleStatesTable *pst,
                  SubsetStatesTable *sst): pst_(pst), sst_(sst){}

  void show(std::ostream& out = std::cout) const {}

  //! Show the sampling space of a single particle
  void show_space(Particle */*p*/,
                  std::ostream& /*out*/ = std::cout) const{};

  //! Fill states as encoded in the node for the input subset of particles
  /** \param[in] particles   a set of particles for which combinations
                             of states should be generated.
      \param[in] states      the dataset to be filled with states.
   */
  void populate_states_of_particles(
              container::ListSingletonContainer *particles,
              Combinations *states) const {
    Pointer<SubsetStates> ss= sst_->get_subset_states(particles);
    for (unsigned int i=0; i< ss->get_number_of_states(); ++i) {
      Ints state= ss->get_state(i);
      CombState *cs= new CombState();
      for (unsigned int j=0; j< state.size(); ++j) {
        cs->add_data_item(particles->get_particle(j), state[j]);
      }
      states->insert(std::make_pair(cs->get_partial_key(particles), cs));
    }
  }

  unsigned int get_number_of_states(Particle *p) const {
    return pst_->get_particle_states(p)->get_number_of_states();
  }

  //! Set the attributes of the particles in the combination to the states
  //! indicated in the combination
  void move2state(const CombState *cs) {
    CombData cd= *cs->get_data();
    for (CombData::const_iterator it= cd.begin(); it != cd.end(); ++it) {
      Pointer<ParticleStates> ps= pst_->get_particle_states(it->first);
      ps->load_state(it->second, it->first);
    }
  }
};

IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO2_DISCRETE_SAMPLER_H */
