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
  //!
  /**
   */
  virtual void show(std::ostream& out = std::cout) const {}
  //! Get a specific attribute value of one of the states
  //! of a specific particle.
  /**
    \param[in] p    The particle
    \param[in] i    The number of the state
    \param[in] key  The key of the attribute
    \return the value of attribute with key of the i'th state of the particle p.
   */
  virtual Float get_state_val(Particle *p, unsigned int i,
                              FloatKey key) const {return 0.0;}
  //! Get the size of the sampling space of a spacific particle
  /**
    \param[in] p A pointer to a particle
     \return the size of the sampling space of the particle
   */
  virtual unsigned int get_space_size(Particle *p) const {return 0;}
  //! Get an attribute key
  /**
    \param[in] p a pointer to a particle
    \param[in] att_num the number of the attribute
    \return the key of the att_num attribute
   */
  virtual FloatKey get_attribute_key(Particle *p,
                                 unsigned int att_num) const
                                 {return FloatKey();}
  //! Get the number of attributes of each state of a specific particle
  /**
    \param[in] p a pointer to a particle
    \return the number of attributes for each state of p
   */
  virtual unsigned int get_number_of_attributes(Particle *p) const
                                               {return 0;}
  //!
  /**
   */
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
  virtual void move2state(const CombState *cs) const {
    IMP::Particle *p;
    for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
      p = it->first;
      for (unsigned int i = 0; i < get_number_of_attributes(p); i++) {
        p->set_value(get_attribute_key(p, i),
                     get_state_val(p, it->second, get_attribute_key(p, i)));
      }
    }
  }
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DISCRETE_SAMPLER_H */
