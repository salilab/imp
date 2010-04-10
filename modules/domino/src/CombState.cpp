/**
 *  \file CombState.cpp \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/domino/CombState.h>
#include <numeric>
#include <climits>

IMPDOMINO_BEGIN_NAMESPACE

unsigned int CombState::get_state_num(Particle *p) const {
  std::stringstream err_msg;
  err_msg << "CombState::get_state_num the particle "
          << p->get_value(node_name_key());
  //  p->show(err_msg);
  err_msg << " is not found in the combstate data : ";
  IMP_INTERNAL_CHECK(data_.find(p) != data_.end(), err_msg.str());
  return data_.find(p)->second;
}

void CombState::show(std::ostream& out) const {
  out << "CombState combination: number of particles : ";
  out << data_.size() << " :: ";
  for (CombData::const_iterator it = data_.begin();
       it != data_.end(); it++) {
    out << it->first->get_value(node_name_key());
    out << " ||| " << it->second << " , ";
  }
  out << " total_score : " << total_score_ << " key:" << key()<<std::endl;
}

//TODO - I copied code here, because I am trying to avoid ParticlesTemp copy
//if possible - check if there is a work around it
CombState *CombState::get_partial(container::ListSingletonContainer *ps) const {
  CombState *part_state = new CombState();
  for (unsigned int i=0;i<ps->get_number_of_particles();i++) {
    Particle *p = ps->get_particle(i);
    IMP_INTERNAL_CHECK(data_.find(p) != data_.end(),
    "CombState::get_partial particle with name "
    << p->get_value(node_name_key()) << " was not found ");
    part_state->add_data_item(p, data_.find(p)->second);
  }
  return part_state;
}
CombState *CombState::get_partial(ParticlesTemp ps) const {
  CombState *part_state = new CombState();
  for (unsigned int i=0;i<ps.size();i++) {
    Particle *p = ps[i];
    IMP_INTERNAL_CHECK(data_.find(p) != data_.end(),
    "CombState::get_partial particle with name "
    << p->get_value(node_name_key()) << " was not found ");
    part_state->add_data_item(p, data_.find(p)->second);
  }
  return part_state;
}

std::string CombState::get_partial_key(
  container::ListSingletonContainer *ps) const {
  CombState *cs = get_partial(ps);
  std::string cs_key = cs->key();
  //delete(cs); //TODO - how to delete something created by IMP_NEW ??
  delete(cs);
  return cs_key;
}
std::string CombState::get_partial_key(
  ParticlesTemp ps) const {
  CombState *cs = get_partial(ps);
  std::string cs_key = cs->key();
  //delete(cs); //TODO - how to delete something created by IMP_NEW ??
  delete(cs);
  return cs_key;
}
IMPDOMINO_END_NAMESPACE
