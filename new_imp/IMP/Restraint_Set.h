/*
 *  Restraint_Set.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_SET_H
#define __IMP_RESTRAINT_SET_H

#include <string>

#include "IMP_config.h"
#include "Restraint.h"

namespace imp
{

// Used to hold a set of related restraints
class IMPDLLEXPORT Restraint_Set
{
public:
  Restraint_Set(std::string name = std::string("no_name"));
  ~Restraint_Set();
  void set_model_data(Model_Data* model_data);

  std::string name(void) {
    return name_;
  }
  void set_name(std::string name) {
    name_ = name;
  }

  // return the score for this restraint or set of restraints
  // ... given the current state of the model
  Float evaluate(bool calc_deriv);

  void add_restraint(Restraint *restraint);
  void add_restraint_set(Restraint_Set *restraint_set);
  // void add_filter(Filter *filter);

  // status
  void set_is_active (const bool is_active);
  bool is_active (void);

  // called when at least one particle has been inactivated
  void check_particles_active (void);

  void show (std::ostream& out = std::cout);

protected:
  // all of the particle data
  Model_Data* model_data_;

  // restraint set is active flag
  bool is_active_;

  // restraint sets have an associated name
  std::string name_;

  std::vector<Restraint_Set *> restraint_sets_;

  // restraints to evaluate
  // these can be accessed with an iterator by a filter.
  // in the case where the restraint is a single simple restraint,
  // ... this vector contains a pointer to itself.
  std::vector<Restraint *> restraints_;

  // Filters
  // std::vector<Filter *> filters_;
};

} // namespace imp

#endif  /* __IMP_RESTRAINT_SET_H */
