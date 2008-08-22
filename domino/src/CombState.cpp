/**
 *  \file CombState.cpp \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "CombState.h"
#include <numeric>
#include <climits>

namespace IMP
{

namespace domino
{

CombState::CombState()
{
  total_score = 0.0;
  data = std::map<Particle *, unsigned int>();
}

const std::string CombState::partial_key(const Particles *ps) const
{
  CombState *cs = get_partial(*ps);
  std::string cs_key = cs->key();
  free(cs);
  return cs_key;
}

CombState *CombState::get_partial(const Particles &ps) const
{
  CombState *part_state = new CombState();
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    Particle *p = *it;
    unsigned int p_index = p->get_index().get_index();
    std::stringstream error_message;
    error_message << "CombState::key particle with index " << p_index
                  << " was not found ";
    IMP_assert(data.find(p) != data.end(), error_message.str());
    (part_state->data)[p] = data.find(p)->second;
  }
  return part_state;
}

const std::string CombState::key() const
{
  std::stringstream s;
  // we assume that the particles in the JNode are stored by their index.
  for (std::map<Particle *, unsigned int>::const_iterator it = data.begin();
       it != data.end(); it++) {
    Particle *p = it->first;
    unsigned int p_index = p->get_index().get_index();
    std::stringstream error_message;
    error_message << "CombState::key particle with index " << p_index
                  << " was not found ";
    IMP_assert(data.find(p) != data.end(), error_message.str());
    s << p_index << ":" << it->second << "_";
  }
  return s.str();
}

bool CombState::is_part(const CombState &other) const
{
  for (std::map<Particle *,
                unsigned int>::const_iterator it = other.data.begin();
       it != other.data.end(); it++) {
    if (data.find(it->first) == data.end()) {
      return false;
    }
    if ((data.find(it->first))->second != it->second) {
      return false;
    }
  }
  return true;
}

void CombState::show(std::ostream& out) const
{
  out << "CombState combination: ";
  for (std::map<Particle *, unsigned int>::const_iterator it = data.begin();
       it != data.end(); it++) {
    out << it->first->get_value(IMP::StringKey("name"))  << "|"
        << it->second << " , ";
  }
  out << " total_score : " << total_score << std::endl;
}

unsigned int CombState::get_state_num(Particle *p)
{
  std::stringstream error_message;
  error_message << "CombState::get_state the particle is not "
                << "part of the corresponding JNode : " << p;
  IMP_assert(data.find(p) != data.end(), error_message.str());
  return data[p];
}

void  CombState::add_data_item(Particle *p, unsigned int val)
{
  std::stringstream error_message;
  error_message << "CombState::add_data_item the particle is already part"
                << "CombState : " << p;
  IMP_assert(data.find(p) == data.end(), error_message.str());
  data[p] = val;
}

void CombState::combine(CombState *other)
{
  for (std::map<Particle *,
                unsigned int>::const_iterator it = other->data.begin();
       it != other->data.end(); it++) {
    Particle *p = it->first;
    std::cout << "CombState::combine working on "
              << p->get_value(IMP::StringKey("name")) << std::endl;
    if (data.find(p) == data.end()) {
      //add particle
      std::cout << "CombState::combine working on "
                << p->get_value(IMP::StringKey("name")) << " ADDING "
                << std::endl;
      data[p] = it->second;
    } else {
      std::stringstream error_message;
      error_message << " CombState::combine the state of particle with name :"
                    << p->get_value(IMP::StringKey("name")) << " is wrong"
                    << " - expect ( " << it->second << " instead of "
                    << data[p] << " )";
      if (data[p] != it->second) {
        std::cout << error_message.str() << std::endl;
      }
      IMP_assert(data[p] == it->second, error_message.str());
    }
  }
}

} // namespace domino

} // namespace IMP
