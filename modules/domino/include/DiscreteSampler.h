/**
 *  \file DiscreteSampler.h   \brief Storage of a discrete sampling space
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPDOMINO_DISCRETE_SAMPLER_H
#define __IMPDOMINO_DISCRETE_SAMPLER_H

#include "IMP/Particle.h"
#include <vector>
#include "domino_exports.h"

IMPDOMINO_BEGIN_NAMESPACE

//! Holds the states of a single particle
/** \note Since this class is header-only, we must not mark it for
          export with IMPDOMINOEXPORT.
 */
class  DiscreteSampler
{
public:
  virtual ~DiscreteSampler(){};

  virtual void show(std::ostream& out = std::cout) const {}
  virtual Float get_state_val(const Particle &p, unsigned int i,
                              FloatKey key) const {return 0.0;}
  virtual unsigned int get_space_size(const Particle &p) const {return 0;}
  virtual FloatKey get_attribute(const Particle &p,
                                 unsigned int att_index) const
                                 {return FloatKey();}
  virtual unsigned int get_number_of_attributes(const Particle &p) const
                                               {return 0;}
  virtual void show_space(const Particle &p,
                          std::ostream& out = std::cout) const {}
};

IMPDOMINO_END_NAMESPACE

#endif  /* __IMPDOMINO_DISCRETE_SAMPLER_H */
