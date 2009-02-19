/**
 *  \file SimpleDiscreteSampler.h   \brief for debugging
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H
#define IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H

#include "config.h"
#include "DiscreteSampler.h"
#include "SimpleDiscreteSpace.h"
#include <IMP/Particle.h>
#include <map>
#include <sstream>


IMPDOMINO_BEGIN_NAMESPACE

class SimpleDiscreteSampler : public DiscreteSampler
{
public:
  SimpleDiscreteSampler(){}
  ~SimpleDiscreteSampler() {}

  void move2state(const CombState *cs) {
    Particle *p;
    SimpleDiscreteSpace *ds;
    const std::vector<FloatKey> *atts;
    for (std::map<Particle *,unsigned int>::const_iterator it
         = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
      p = it->first;
      ds = data.find(p)->second;
      atts = ds->get_att_keys();
      for (std::vector<FloatKey>::const_iterator k_iter = atts->begin();
        k_iter != atts->end(); k_iter++) {
        p->set_value(*k_iter,ds->get_state_val(it->second, *k_iter));
      }
    }
  }
  void populate_states_of_particles(Particles *particles,
                        std::map<std::string, CombState *> *states) const
  {
    Int num_states = 1;
    for (Particles::const_iterator it = particles->begin();
         it != particles->end(); it++) {
      num_states *= data.find(*it)->second->get_number_of_states();
    }
    Int global_iterator, global_index;
    CombState *calc_state;
    Particle* p;
    Int sample_size;
    for (Int state_index = 0;state_index < num_states; state_index++) {
      calc_state = new CombState();
      global_iterator = num_states;
      global_index = state_index;
      for (Particles::const_iterator it = particles->begin();
           it != particles->end(); it++) {
        p = *it;
        sample_size = data.find(p)->second->get_number_of_states();
        global_iterator /= sample_size;
        calc_state->add_data_item(p, global_index / global_iterator);
        global_index -= (global_index / global_iterator) * global_iterator;
      }
      (*states)[calc_state->partial_key(particles)] = calc_state;
    }
  }
  virtual void show_space(Particle *p,
                 std::ostream& out = std::cout) const {}

  void show(std::ostream& out) const {
    out << "================ show sampling spaces ============== " << std::endl;
    for (std::map<const Particle *,SimpleDiscreteSpace *>::const_iterator it
         = data.begin(); it != data.end(); it++) {
      out << " space for particle with index: "
          << it->first->get_index() << " is : ";
      it->second->show(out);
      out << std::endl;
    }
    out << "=================================================== " << std::endl;
  }

  Float get_state_val(Particle *p, unsigned int i, FloatKey key) const {
    return data.find(p)->second->get_state_val(i, key);
  }
  unsigned int get_space_size(Particle *p) const {
    return data.find(p)->second->get_number_of_states();
  }
  FloatKey get_attribute_key(Particle *p, unsigned int att_index) const {
    return (*(data.find(p)->second->get_att_keys()))[att_index];
  }
  unsigned int get_number_of_attributes(Particle *p) const {
    return data.find(p)->second->get_number_of_attributes();
  }
  void add_space(const Particle &p, SimpleDiscreteSpace &sds) {
    data[&p] = &sds;
  }

protected:
  std::map<const Particle *, SimpleDiscreteSpace *> data;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SIMPLE_DISCRETE_SAMPLER_H */
