/**
 * \file  DiscreteSet.cpp
 * \brief Holds a discrete sampling space.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/domino/DiscreteSet.h"

IMPDOMINO_BEGIN_NAMESPACE

DiscreteSet::DiscreteSet(const std::vector<FloatKey> &atts) {
  atts_.insert(atts_.begin(),atts.begin(),atts.end());
}
void DiscreteSet::add_state(Particle* p) {
  //check that the particle holds of the necessary attributes
  //TODO - this loop should probably not be there all of the times.
  std::stringstream err_msg;
  for (std::vector<FloatKey>::iterator it = atts_.begin(); it != atts_.end();
       it++) {
    err_msg.clear();
    err_msg <<"DiscreteSet::add_state The particle does not have the attribute";
    it->show(err_msg);
    IMP_assert(p->has_attribute(*it),err_msg.str());
    p->get_value(*it);
  }
  states_.push_back(p);
}

Particle * DiscreteSet::get_state(long state_ind) const {
  std::stringstream err_msg;
  err_msg <<"DiscreteSet::get_state the input state index: " << state_ind;
  err_msg << " is out of range ( " << states_.size() << " ) " ;
  IMP_assert(static_cast<unsigned int>(state_ind)<states_.size(),err_msg.str());
  return states_[state_ind];
}

Float DiscreteSet::get_state_val(long state_ind, IMP::FloatKey key) const {
  std::stringstream err_msg;
  err_msg <<"DiscreteSet::get_state_val the input state index: " << state_ind;
  err_msg << " is out of range ( " << states_.size() << " ) " ;
  IMP_assert(state_ind<states_.size(),err_msg.str());
  return states_[state_ind]->get_value(key);
  // if the key does not exist Particle will raise an exception
}


void DiscreteSet::show(std::ostream& out) const {
  out<<"DiscreteSet::show number of states : ";
  out<<get_number_of_states()<<std::endl;
  for(Particles::const_iterator it1 = states_.begin();
      it1 != states_.end(); it1++) {
    out << "state " << it1-states_.begin() << " :: ";
    for(std::vector<FloatKey>::const_iterator it2 = atts_.begin();
        it2 != atts_.end(); it2++) {
      out<<*it2<<"|"<<(*it1)->get_value(*it2)<<",";
    }
    out<<std::endl;
  }
}


IMPDOMINO_END_NAMESPACE
