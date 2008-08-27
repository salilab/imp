/**
 *  \file DiscreteSampler.h   \brief Storage of a discrete sampling space
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_DISCRETE_SAMPLER_H
#define __IMP_DISCRETE_SAMPLER_H

#include "IMP/Particle.h"
#include <vector>

namespace IMP
{

namespace domino
{

//! Holds the states of a single particle
/** \note Since this class is header-only, we must not mark it for
          export with IMPDOMINOEXPORT.
 */
class DiscreteSampler
{
public:
  virtual ~DiscreteSampler() {};

  virtual void show(std::ostream& out = std::cout) const {}
  virtual Float get_state_val(const Particle &p, unsigned int i,
                              FloatKey key) const = 0;
  virtual unsigned int get_space_size(const Particle &p) const = 0;
  virtual FloatKey get_attribute(const Particle &p,
                                 unsigned int att_index) const = 0;
  virtual unsigned int get_number_of_attributes(const Particle &p) const = 0;
  virtual void show_space(const Particle &p,
                          std::ostream& out = std::cout) const {}
  //! Shallow copy of the sampler instance
  virtual void shallow_copy(DiscreteSampler *copy_ds) const {}
  virtual void change_mapping(const Particles &orig_particles,
                              const Particles &shuffled_particles) {}
};

} // namespace domino

} // namespace IMP

#endif  /* __IMP_DISCRETE_SAMPLER_H */
