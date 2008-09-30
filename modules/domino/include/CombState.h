/**
 *  \file CombState.h   \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPDOMINO_COMB_STATE_H
#define __IMPDOMINO_COMB_STATE_H

#include "domino_exports.h"
#include "DiscreteSampler.h"
#include "IMP/Particle.h"
#include "IMP/Restraint.h"
#include "IMP/restraints/RestraintSet.h"
#include "IMP/decorators/XYZDecorator.h"
#include <vector>
#include <sstream>
#include <algorithm>

namespace IMP
{
namespace domino
{

class CombState
{
public:
  //! Constructor
  CombState() {
  total_score = 0.0;
  data = std::map<Particle *, unsigned int>();

  }
  //! Copy constructor
  CombState(const CombState &other){
  total_score = 0.0;
  data = std::map<Particle *, unsigned int>();

  for(std::map<Particle*,unsigned int>::const_iterator it = other.data.begin();
      it != other.data.end(); it++) {
    data[it->first]=it->second;
  }
  total_score = other.total_score;

  }
  void add_data_item(Particle *p, unsigned int val) {
  std::stringstream error_message;
  error_message << "CombState::add_data_item the particle is already part"
                << "CombState : " << p;
  IMP_assert(data.find(p) == data.end(), error_message.str());
  data[p] = val;

  }
  unsigned int get_state_num(Particle *p) {
  std::stringstream error_message;
  error_message << "CombState::get_state the particle is not "
                << "part of the corresponding JNode : " << p;
  IMP_assert(data.find(p) != data.end(), error_message.str());
  return data[p];

  }
  const std::string key() const {
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
  const std::string partial_key(const Particles *ps) const {
  CombState *cs = get_partial(*ps);
  std::string cs_key = cs->key();
  delete(cs);
  return cs_key;

  }
  CombState *get_partial(const Particles &ps) const {
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
  //  void add_term(Restraint *r);

  void update_total_score(float old_val, float new_val) {
    total_score += new_val - old_val;
  }

  float get_total_score() const {
    return total_score;
  }

  //! Checks if the other state is part of this state
  /** \param[in] other the other state
   */
  bool is_part(const CombState &other) const {
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

  //  CombState* find_minimum() const;
  void show(std::ostream& out = std::cout) const {
  out << "CombState combination: ";
  for (std::map<Particle *, unsigned int>::const_iterator it = data.begin();
       it != data.end(); it++) {
    out << it->first->get_value(IMP::StringKey("name"))  << "|"
        << it->second << " , ";
  }
  out << " total_score : " << total_score << std::endl;

  }

  //! Combine the combination encoded in other
  /** \param[in] other the other combination
   */
  void combine(CombState *other) {
  for (std::map<Particle *,
                unsigned int>::const_iterator it = other->data.begin();
       it != other->data.end(); it++) {
    Particle *p = it->first;
    if (data.find(p) == data.end()) {
      //add particle
      data[p] = it->second;
    } else {
      std::stringstream error_message;
      error_message << " CombState::combine the state of particle with name :"
                    << p->get_value(IMP::StringKey("name")) << " is wrong"
                    << " - expect ( " << it->second << " instead of "
                    << data[p] << " )";
      IMP_assert(data[p] == it->second, error_message.str());
    }
  }

  }
  const std::map<Particle *, unsigned int> & get_data() const {
    return data;
  }
  void move2state(DiscreteSampler *ds) const {
    Particle *p;
    FloatKey att;
    Float val;
    for (std::map<Particle *, unsigned int>::const_iterator it =  data.begin();
      it != data.end();it++){
      p = it->first;
      for (unsigned int i = 0; i < ds->get_number_of_attributes(*p); i++) {
        att = ds->get_attribute(*p, i);
        val = ds->get_state_val(*p,it->second,att );
        p->set_value(att,val);
      }
    }
  }
protected:
  std::map<Particle *, unsigned int> data; //data[p] = for particle p
  // the state is state number  data[p] from the assigned states.
  float total_score;
};


} // namespace domino

} // namespace IMP

#endif  /* __IMPDOMINO_COMB_STATE_H */
