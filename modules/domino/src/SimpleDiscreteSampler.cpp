/**
 *  \file SimpleDiscreteSampler.cpp   \brief for debugging
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/domino/SimpleDiscreteSampler.h>

IMPDOMINO_BEGIN_NAMESPACE
void SimpleDiscreteSampler::show(std::ostream& out) const {
  out << "================ show sampling spaces ============== " << std::endl;
  for (std::map<const Particle *,SimpleDiscreteSpace *>::const_iterator it
       = data_.begin(); it != data_.end(); it++) {
    out << " space for particle with index: "
        << it->first->get_value(node_name_key()) << " is : ";
    it->second->show(out);
    out << std::endl;
  }
  out << "=================================================== " << std::endl;
}


void SimpleDiscreteSampler::move2state(const CombState *cs) {
    Particle *p;
    SimpleDiscreteSpace *ds;
    FloatKeys atts;
    for (std::map<Particle *,unsigned int>::const_iterator
         it = cs->get_data()->begin();it != cs->get_data()->end(); it++) {
      p = it->first;
      ds = data_.find(p)->second;
      atts = ds->get_att_keys();
      for (std::vector<FloatKey>::const_iterator k_iter = atts.begin();
        k_iter != atts.end(); k_iter++) {
        IMP_LOG(VERBOSE,"particle : " << p->get_value(node_name_key())
                << "setting value for: " << *k_iter <<" to: "
                << it->second <<std::endl);
        p->set_value(*k_iter,ds->get_state_val(it->second, *k_iter));
      }
    }
  }


void SimpleDiscreteSampler::populate_states_of_particles(
   container::ListSingletonContainer *particles,
   std::map<std::string, CombState *> *states) const {
    Int num_states = 1;
    for (unsigned int i=0;i<particles->get_number_of_particles();i++){
      Particle *p=particles->get_particle(i);
      num_states *= data_.find(p)->second->get_number_of_states();
    }
    Int global_iterator, global_index;
    //CombState *calc_state;
    Particle* p;
    Int sample_size;
    for (Int state_index = 0;state_index < num_states; state_index++) {
      CombState *calc_state = new CombState();
      global_iterator = num_states;
      global_index = state_index;
      for (unsigned int i=0;i<particles->get_number_of_particles();i++){
        p=particles->get_particle(i);
        sample_size = data_.find(p)->second->get_number_of_states();
        global_iterator /= sample_size;
        calc_state->add_data_item(p, global_index / global_iterator);
        global_index -= (global_index / global_iterator) * global_iterator;
      }
      (*states)[calc_state->get_partial_key(particles)] = calc_state;
    }
  }


IMPDOMINO_END_NAMESPACE
