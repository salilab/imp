/**
 *  \file CombState.h   \brief Handles all functionalities of a single state
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_COMB_STATE_H
#define __IMP_COMB_STATE_H

#include "domino_config.h"
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
  CombState();

  void add_data_item(Particle *p, unsigned int val);

  unsigned int get_state_num(Particle *p);
  const std::string key() const;
  const std::string partial_key(const Particles *ps) const;
  CombState *get_partial(const Particles &ps) const;
  void add_term(Restraint *r);

  void update_total_score(float old_val, float new_val) {
    total_score += new_val - old_val;
  }

  float get_total_score() const {
    return total_score;
  }

  //! Checks if the other state is part of this state
  /** /param[in] other the other state
   */
  bool is_part(const CombState &other) const;

  CombState* find_minimum() const;
  void show(std::ostream& out = std::cout) const;

  //! Combine the combination encoded in other
  /** /param[in] other the other combination
   */
  void combine(CombState *other);
  const std::map<Particle *, unsigned int> & get_data() const {
    return data;
  }
protected:
  std::map<Particle *, unsigned int> data; //data[p] = for particle p
  // the state is state number  data[p] from the assigned states.
  float total_score;
};

} // namespace domino

} // namespace IMP

#endif  /* __IMP_COMB_STATE_H */
