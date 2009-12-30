/**
 *  \file CombState.h   \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_COMB_STATE_H
#define IMPDOMINO_COMB_STATE_H

#include "config.h"
#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include "IMP/base_types.h"
IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

// backwards compatibility functions
//TODO - I think I solved the problem, see if the new version is stable
//and then remove evil hack
inline  unsigned int particle_index(Particle *p) {
  Model::ParticleIterator pit= p->get_model()->particles_begin();
  unsigned int ret=0;
  while (*pit != p) {
    ++pit;
    ++ret;
    IMP_INTERNAL_CHECK(pit != p->get_model()->particles_end(),
               "Particle not found");
    }

  /****************************************************************

     EVIL HACK. +1 is needed to get the tests to pass, but I suspect
     the tests are broken. It is awaiting discussion with Keren.

  ****************************************************************/
  //return ret+1;
  return ret;
}
IMPDOMINO_END_INTERNAL_NAMESPACE

IMPDOMINO_BEGIN_NAMESPACE
// defined in RestraintGraph
IMPDOMINOEXPORT StringKey node_name_key();

typedef std::map<Particle *, unsigned int> CombData;
//! Holds a combination of states for each particle in the restraint graph
/**
Holds an index for fast merge and split operations ans well as for
is_subset queries
 **/
class IMPDOMINOEXPORT CombState
{
public:
  //! Constructor
  CombState() {
  total_score_ = 0.0;
  }
  //! Copy constructor
  CombState(const CombState &other){
  total_score_ = 0.0;
  //  data_ = CombData();
  for(CombData::const_iterator it = other.data_.begin();
      it != other.data_.end(); it++) {
    data_[it->first]=it->second;
  }
  total_score_ = other.total_score_;
  }
  void add_data_item(Particle *p, unsigned int val) {
    IMP_INTERNAL_CHECK(data_.find(p) == data_.end(),
             "CombState::add_data_item the particle is already part"
             << "CombState : " << p <<std::endl);
    data_[p] = val;
  }

  bool has_particle(Particle *p) {
    return data_.find(p) != data_.end();
  }

  unsigned int get_state_num(Particle *p) const;

  const std::string key() const {
  std::stringstream s;
  // we assume that the particles in the JNode are stored by their index.
  for (CombData::const_iterator it = data_.begin();
       it != data_.end(); it++) {
    Particle *p = it->first;
    unsigned int p_index= internal::particle_index(p);
    IMP_INTERNAL_CHECK(data_.find(p) != data_.end(),
               "CombState::key particle with index " << p_index
               << " was not found ");
    s << p_index << ":" << it->second << "_";
  }
  return s.str();

  }
  const std::string partial_key(const Particles *ps) const;
  CombState *get_partial(const Particles &ps) const;
  //  void add_term(Restraint *r);

  //Update the total score
  /**
    \param[in]  old_val the value to substracte
    \param[in] new_val the value to add
    \note the total score is updated to be current - old_val + new_val
   */
  void update_total_score(float old_val, float new_val) {
    total_score_ += new_val - old_val;
  }
  //Update the total score
  /**
  \param[in] val the total score is updated to be val
  */
  void set_total_score(float val) {
    total_score_ = val;
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
  IMP_LOG(VERBOSE,"before combination the size is : " <<
                  data_.size() << std::endl);
  for (CombData::const_iterator it = other->data_.begin();
       it != other->data_.end(); it++) {
    Particle *p = it->first;
    if (data_.find(p) == data_.end()) {
      //add particle
      data_[p] = it->second;
    } else {
      std::stringstream error_message;
      IMP_INTERNAL_CHECK(data_[p] == it->second,
                 " CombState::combine the state of particle with name :"
                 << p->get_value(node_name_key()) << " is wrong"
                 << " - expect ( " << it->second << " instead of "
                 << data_[p] << " )"<<std::endl);

    }
  }
  IMP_LOG(VERBOSE,"after combination the size is : "
                  << data_.size() << std::endl);
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
typedef std::map<std::string, float> CombinationValues;
typedef std::vector<CombState*> CombStates;

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_COMB_STATE_H */
