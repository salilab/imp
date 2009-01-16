/**
 *  \file SimpleDiscreteSpace.h   \brief for debugging
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_SIMPLE_DISCRETE_SPACE_H
#define IMPDOMINO_SIMPLE_DISCRETE_SPACE_H

#include "config.h"
#include "DiscreteSampler.h"
#include "DiscreteSet.h"
#include <IMP/Particle.h>
#include <map>
#include <sstream>

#define KEY_OPT "OPT"

IMPDOMINO_BEGIN_NAMESPACE

class SimpleDiscreteSpace : public DiscreteSet
{
public:
  SimpleDiscreteSpace(int number_of_states):m_(){
  atts_.push_back(FloatKey(KEY_OPT));
  Particle * p;
  for (int j = 0;j < number_of_states;j++) {
    p = new Particle();
    m_.add_particle(p);
    p->add_attribute(KEY_OPT,j,true);
    states_.push_back(p);
  }
}

protected:
  Model m_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SIMPLE_DISCRETE_SPACE_H */
