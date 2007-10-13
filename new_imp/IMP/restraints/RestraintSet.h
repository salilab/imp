/*
 *  RestraintSet.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_SET_H
#define __IMP_RESTRAINT_SET_H

#include <string>

#include "../IMP_config.h"
#include "Restraint.h"

namespace IMP
{

// Used to hold a set of related restraints
class IMPDLLEXPORT RestraintSet: public Restraint
{
public:
  RestraintSet(const std::string& name=std::string());
  ~RestraintSet();

  /** Return the score for this restraint or set of restraints
      given the current state of the model */
  virtual Float evaluate(bool calc_deriv);

  //! The type to use to retreive a restraint
  typedef int RestraintIndex;

  /** Add restraint to the restraint set. */
  RestraintIndex add_restraint(Restraint *restraint);

  /** Access a restraint in this set*/
  Restraint *get_restraint(RestraintIndex i) const;

  /** Return the total number of restraints*/
  unsigned int number_of_restraints() const {
    return restraints_.size();
  }

  // called when at least one particle has been inactivated
  virtual void check_particles_active (void);

  void show (std::ostream& out = std::cout) const;

protected:

   // restraints to evaluate
  // these can be accessed with an iterator by a filter.
  // in the case where the restraint is a single simple restraint,
  // ... this vector contains a pointer to itself.
  std::vector<Restraint *> restraints_;

  // Filters
  // std::vector<Filter *> filters_;
};

} // namespace IMP

#endif  /* __IMP_RESTRAINT_SET_H */
