/**
 *  \file SimpleDiscreteSampler.h   \brief for debugging
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H
#define IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H

#include "domino_config.h"
#include "DiscreteSampler.h"
#include "SimpleDiscreteSpace.h"
#include <IMP/Particle.h>
#include <map>
#include <sstream>


IMPDOMINO_BEGIN_NAMESPACE

class IMPDOMINOEXPORT SimpleDiscreteSampler : public DiscreteSampler
{
public:
  SimpleDiscreteSampler(){}
  void move2state(const CombState *cs);
  void populate_states_of_particles(
         container::ListSingletonContainer *particles,
         std::map<std::string, CombState *> *states) const;
  void show_space(Particle */*p*/,
                  std::ostream& /*out*/ = std::cout) const{}

  void show(std::ostream& out=std::cout) const;

  Float get_state_val(Particle *p, unsigned int i, FloatKey key) const {
    return data_.find(p)->second->get_state_val(i, key);
  }

  unsigned int get_space_size(Particle *p) const {
    return data_.find(p)->second->get_number_of_states();
  }

  FloatKey get_attribute_key(Particle *p, unsigned int att_index) const {
    return (data_.find(p)->second->get_att_keys())[att_index];
  }

  unsigned int get_number_of_attributes(Particle *p) const {
    return data_.find(p)->second->get_number_of_attributes();
  }

  void add_space(const Particle* p, SimpleDiscreteSpace &sds) {
    data_[p] = &sds;
  }
  DiscreteSet* get_space(Particle *p) const {
  IMP_INTERNAL_CHECK(data_.find(p) != data_.end(),
          "Particle "<<p->get_name() <<
          "was not found in SimpleDiscreteSampler" <<std::endl);
  return data_.find(p)->second;
  }
protected:
  std::map<const Particle *, SimpleDiscreteSpace *> data_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H */
