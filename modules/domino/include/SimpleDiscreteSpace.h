/**
 *  \file SimpleDiscreteSpace.h   \brief for debugging
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_SIMPLE_DISCRETE_SPACE_H
#define IMPDOMINO_SIMPLE_DISCRETE_SPACE_H

#include "config.h"
#include "DiscreteSampler.h"

#include <IMP/Particle.h>

#include <map>
#include <sstream>

#define KEY_OPT "OPT"

IMPDOMINO_BEGIN_NAMESPACE

class SimpleDiscreteSpace
{
public:
  SimpleDiscreteSpace(int number_of_states) {
    attributes =  std::vector<FloatKey>();
    attributes.push_back(FloatKey(KEY_OPT));
    for (int j = 0;j < number_of_states;j++) {
      values.push_back(std::map<FloatKey, float>());
      values[values.size()-1][FloatKey(KEY_OPT)] = j;
    }
  }
  ~SimpleDiscreteSpace() {};
  float get_state_val(unsigned int i, FloatKey key) const {
    return values[i].find(key)->second;
  }
  unsigned int size() const {
    return values.size();
  }
  const std::vector<FloatKey> & get_attributes() const {
    return attributes;
  }
  void show(std::ostream& out = std::cout) const {
    for (std::vector<std::map<FloatKey, float> >::const_iterator it
         = values.begin(); it != values.end(); it++) {
      out << " || ";
      for (std::map<FloatKey, float>::const_iterator it2 = it->begin();
           it2 != it->end();it2++) {
        out << " ( " << it2->first << " , " << it2->second << " ) ";
      }
    }
  }
protected:
  std::vector<std::map<FloatKey, float> > values;
  std::vector<FloatKey> attributes;

};

class SimpleDiscreteSampler : public DiscreteSampler
{
public:
  SimpleDiscreteSampler() {
    data = std::map<const Particle *, SimpleDiscreteSpace *>();
  }
  ~SimpleDiscreteSampler() {
  }
  void show(std::ostream& out = std::cout) const {
    out << "================ show sampling spaces ============== " << std::endl;
    for (std::map<const Particle *,
                  SimpleDiscreteSpace *>::const_iterator it = data.begin();
         it != data.end(); it++) {
      out << " space for particle with index: "
          << it->first->get_index().get_index() << " is : ";
      it->second->show(out);
      out << std::endl;
    }
    out << "=================================================== " << std::endl;
  }
  Float get_state_val(Particle *p, unsigned int i, FloatKey key) const {
    return data.find(p)->second->get_state_val(i, key);
  }
  unsigned int get_space_size(Particle *p) const {
    return data.find(p)->second->size();
  }
  FloatKey get_attribute_key(Particle *p, unsigned int att_index) const {
    return data.find(p)->second->get_attributes()[att_index];
  }
  unsigned int get_number_of_attributes(Particle *p) const {
    return data.find(p)->second->get_attributes().size();
  }
  void show_space(Particle *p, std::ostream& out = std::cout) const {
    data.find(p)->second->show(out);
  }
  void add_space(const Particle &p, SimpleDiscreteSpace &sds) {
    data[&p] = &sds;
  }

  void populate_states_of_particles(Particles *particles,
      std::map<std::string, CombState *> *states) const
  {
    Int num_states = 1;
    for (Particles::const_iterator it = particles->begin();
         it != particles->end(); it++) {
      num_states *= data.find(*it)->second->size();
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
        sample_size = data.find(p)->second->size();
        global_iterator /= sample_size;
        calc_state->add_data_item(p, global_index / global_iterator);
        global_index -= (global_index / global_iterator) * global_iterator;
      }
      (*states)[calc_state->partial_key(particles)] = calc_state;
    }
  }

protected:
  std::map<const Particle *, SimpleDiscreteSpace *> data;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SIMPLE_DISCRETE_SPACE_H */
