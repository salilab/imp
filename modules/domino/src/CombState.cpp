/**
 *  \file CombState.cpp \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/domino/CombState.h>
#include <numeric>
#include <climits>

IMPDOMINO_BEGIN_NAMESPACE

unsigned int CombState::get_state_num(Particle *p) {
  std::stringstream err_msg;
  err_msg << "CombState::get_state_num the particle " << p->get_index();
  //  p->show(err_msg);
  err_msg << " is not found in the combstate data : ";
  IMP_assert(data.find(p) != data.end(), err_msg.str());
  return data[p];
}

void CombState::show(std::ostream& out) const {
  StringKey name("name");
  out << "CombState combination: number of particles : ";
  out << data.size() << " :: ";
  for (std::map<Particle *, unsigned int>::const_iterator it = data.begin();
       it != data.end(); it++) {
    if (it->first->has_attribute(name)){
      out << it->first->get_value(IMP::StringKey("name"));
      out << " ||| " << it->second << " , ";
    }
    else {
      out <<it->first->get_index().get_index()<< " ||| " <<it->second<< " , ";
    }
  }
  out << " total_score : " << total_score << std::endl;
}


CombState *CombState::get_partial(const Particles &ps) const {
  CombState *part_state = new CombState();
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    Particle *p = *it;
    std::stringstream error_message;
    error_message << "CombState::get_partial particle with index ";
    error_message << p->get_index().get_index() << " was not found ";
    IMP_assert(data.find(p) != data.end(), error_message.str());
    (part_state->data)[p] = data.find(p)->second;
  }
  return part_state;
}


IMPDOMINO_END_NAMESPACE
