/**
 *  \file CombState.cpp \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/domino/CombState.h>
#include <numeric>
#include <climits>

IMPDOMINO_BEGIN_NAMESPACE

unsigned int CombState::get_state_num(Particle *p) {
  std::stringstream err_msg;
  err_msg << "CombState::get_state_num the particle " << p->get_name();
  //  p->show(err_msg);
  err_msg << " is not found in the combstate data : ";
  IMP_assert(data_.find(p) != data_.end(), err_msg.str());
  return data_[p];
}

void CombState::show(std::ostream& out) const {
  out << "CombState combination: number of particles : ";
  out << data_.size() << " :: ";
  for (CombData::const_iterator it = data_.begin();
       it != data_.end(); it++) {
    if (it->first->has_attribute(node_name_key())){
      out << it->first->get_value(node_name_key());
      out << " ||| " << it->second << " , ";
    }
    else {
      out <<it->first->get_name() << " ||| " <<it->second<< " , ";
    }
  }
  out << " total_score : " << total_score_ << std::endl;
}


CombState *CombState::get_partial(const Particles &ps) const {
  CombState *part_state = new CombState();
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    Particle *p = *it;
    IMP_assert(data_.find(p) != data_.end(),
    "CombState::get_partial particle with index "
    << p->get_name() << " was not found ");
    (part_state->data_)[p] = data_.find(p)->second;
  }
  return part_state;
}


IMPDOMINO_END_NAMESPACE
