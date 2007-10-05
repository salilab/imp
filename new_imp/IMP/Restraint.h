/*
 *  Restraint.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#if !defined(__restraint_h)
#define __restraint_h 1

#include "IMP_config.h"

namespace imp
{

// Abstract class for representing restraints
class IMPDLLEXPORT Restraint
{
public:
  Restraint();
  virtual ~Restraint();
  void set_model_data(Model_Data* model_data);

  // return the score for this restraint or set of restraints
  // ... given the current state of the model
  virtual Float evaluate(bool calc_deriv) = 0;

  // status
  void set_is_active(const bool active);
  bool is_active(void);

  // called when at least one particle has been inactivated
  virtual void check_particles_active(void);

  virtual void show(std::ostream& out = std::cout);
  virtual std::string version(void) {
    return "unknown";
  }
  virtual std::string last_modified_by(void) {
    return "unknown";
  }

protected:
  // all of the particle data
  Model_Data* model_data_;

  // restraint is active if active_ AND particles_active_
  // true if restraint has not been deactivated
  // if it is not active, evaluate should not be called
  bool is_active_;
  // true if all particles that restraint uses are active
  bool are_particles_active_;

  // shouldn't be necessary, but keep around for debugging
  std::vector<Particle*> particles_;
};

} // namespace imp

#endif
