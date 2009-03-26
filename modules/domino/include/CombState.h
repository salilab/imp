/**
 *  \file CombState.h   \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_COMB_STATE_H
#define IMPDOMINO_COMB_STATE_H

#include "config.h"
//#include "DiscreteSampler.h"

#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <vector>
#include <sstream>
#include <algorithm>

IMPDOMINO_BEGIN_NAMESPACE
// defined in RestraintGraph
IMPDOMINOEXPORT StringKey node_name_key();

typedef std::map<Particle *, unsigned int> CombData;
class IMPDOMINOEXPORT CombState
{
public:
  //! Constructor
  CombState() {
  total_score_ = 0.0;
  data_ = CombData();

  }
  //! Copy constructor
  CombState(const CombState &other){
  total_score_ = 0.0;
  data_ = CombData();

  for(CombData::const_iterator it = other.data_.begin();
      it != other.data_.end(); it++) {
    data_[it->first]=it->second;
  }
  total_score_ = other.total_score_;
  }
  void add_data_item(Particle *p, unsigned int val) {
  std::stringstream error_message;
  error_message << "CombState::add_data_item the particle is already part"
                << "CombState : " << p;
  IMP_assert(data_.find(p) == data_.end(), error_message.str());
  data_[p] = val;

  }
  bool has_particle(Particle *p) {
    return data_.find(p) != data_.end();
  }

  unsigned int get_state_num(Particle *p);

  const std::string key() const {
  std::stringstream s;
  // we assume that the particles in the JNode are stored by their index.
  for (CombData::const_iterator it = data_.begin();
       it != data_.end(); it++) {
    Particle *p = it->first;
    IMP_assert(data_.find(p) != data_.end(),
               "CombState::key particle with index " << p->get_name()
               << " was not found ");
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
  CombState *get_partial(const Particles &ps) const;
  //  void add_term(Restraint *r);

  void update_total_score(float old_val, float new_val) {
    total_score_ += new_val - old_val;
  }

  Float get_total_score() const {
    return total_score_;
  }

  //! Checks if the other state is part of this state
  /** \param[in] other the other state
   */
  bool is_part(const CombState &other) const {
  for (CombData::const_iterator it = other.data_.begin();
       it != other.data_.end(); it++) {
    if (data_.find(it->first) == data_.end()) {
      return false;
    }
    if ((data_.find(it->first))->second != it->second) {
      return false;
    }
  }
  return true;

  }

  //  CombState* find_minimum() const;
  void show(std::ostream& out = std::cout) const;

  //! Combine the combination encoded in other
  /** \param[in] other the other combination
   */
  void combine(CombState *other) {
  for (CombData::const_iterator it = other->data_.begin();
       it != other->data_.end(); it++) {
    Particle *p = it->first;
    if (data_.find(p) == data_.end()) {
      //add particle
      data_[p] = it->second;
    } else {
      std::stringstream error_message;
      error_message << " CombState::combine the state of particle with name :"
                    << p->get_value(node_name_key()) << " is wrong"
                    << " - expect ( " << it->second << " instead of "
                    << data_[p] << " )";
      IMP_assert(data_[p] == it->second, error_message.str());
    }
  }

  }
  const CombData * get_data() const {
    return &data_;
  }
protected:
  CombData data_; //data[p] = for particle p
  // the state is state number  data[p] from the assigned states.
  Float total_score_;
};


typedef std::map<std::string, CombState *> Combinations;

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_COMB_STATE_H */
