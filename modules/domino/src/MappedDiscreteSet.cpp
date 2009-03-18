/**
 * \file  MappedDiscreteSet.cpp
 * \brief Holds a mapped discrete sampling space.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/domino/MappedDiscreteSet.h"

IMPDOMINO_BEGIN_NAMESPACE
MappedDiscreteSet::MappedDiscreteSet(Particles *ps_target)
{
  for (Particles::const_iterator it = ps_target->begin();
       it != ps_target->end();it++) {
    states_map_[*it] = std::vector<Particle *>();
  }
}
MappedDiscreteSet::MappedDiscreteSet(Particles *ps_target,
        const std::vector<FloatKey> &atts) : DiscreteSet(atts){
  for (Particles::const_iterator it = ps_target->begin();
       it != ps_target->end();it++) {
    states_map_[*it] = std::vector<Particle *>();
  }
}
void MappedDiscreteSet::add_mapped_state(Particle* p_target,Particle *p_src)
{
  IMP_assert(states_map_.find(p_target) != states_map_.end(),
     "The model particle is not part of the mapping particles.");
  states_map_[p_target].push_back(p_src);
}

long MappedDiscreteSet::get_number_of_mapped_states(Particle *p_target) const
{
  return states_map_.find(p_target)->second.size();
}

Particle * MappedDiscreteSet::get_mapped_state(Particle *p_target,
                                               long state_ind) const {
  std::stringstream err_msg;
  err_msg <<"MappedDiscreteSet::get_mapped_state the input state index: ";
  err_msg << state_ind << " is out of range ( " << states_.size() << " ) ";
  IMP_assert(state_ind<states_map_.find(p_target)->second.size(),err_msg.str());
  return (states_map_.find(p_target)->second)[state_ind];
}

Float MappedDiscreteSet::get_mapped_state_val(Particle* p_target,
                           long state_ind, IMP::FloatKey key) const {
  std::stringstream err_msg;
  err_msg <<"MappedDiscreteSet::get_mapped_state_val the input state index: ";
  err_msg <<state_ind <<" for particle : "<<p_target->get_index();
  err_msg <<" is out of range ( " << get_number_of_mapped_states(p_target);
  err_msg << " ) " <<std::endl;
  IMP_assert(state_ind<get_number_of_mapped_states(p_target),err_msg.str());
  return (states_map_.find(p_target)->second)[state_ind]->get_value(key);
  // if the key does not exist Particle will raise an exception
}

void MappedDiscreteSet::show(std::ostream& out) const {
  out<<"MappedDiscreteSet::show number of states : ";
  out<<get_number_of_states()<<std::endl;
  for(std::map<Particle*, std::vector<Particle *> >::const_iterator
      it1 = states_map_.begin(); it1 != states_map_.end(); it1++) {
    out << "states for : ";
    it1->first->show(out);
    out << " :: " << std::endl;
  }
}
IMPDOMINO_END_NAMESPACE
