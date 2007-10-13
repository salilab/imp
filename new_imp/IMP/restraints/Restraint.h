/*
 *  Restraint.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_H
#define __IMP_RESTRAINT_H

#include <vector>
#include <iostream>

#include "../IMP_config.h"
#include "../ModelData.h"
#include "../ScoreFunc.h"

namespace IMP
{

// Abstract class for representing restraints
class IMPDLLEXPORT Restraint
{
public:
  Restraint(std::string name=std::string());
  virtual ~Restraint();
  void set_model_data(ModelData* model_data);

  // return the score for this restraint or set of restraints
  // ... given the current state of the model
  virtual Float evaluate(bool calc_deriv) = 0;

  // status
  void set_is_active(const bool active);
  bool is_active(void) const;

  // called when at least one particle has been inactivated
  virtual void check_particles_active(void);

  virtual void show(std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "unknown";
  }
  virtual std::string last_modified_by(void) const {
    return "unknown";
  }
  //! Get the name of the restraint
  const std::string& get_name() const{return name_;}
  //! Set the name of the restraint
  void set_name(const std::string &name){ name_=name;}
protected:
  // all of the particle data
  ModelData* model_data_;

  // restraint is active if active_ AND particles_active_
  // true if restraint has not been deactivated
  // if it is not active, evaluate should not be called
  bool is_active_;
  // true if all particles that restraint uses are active
  bool are_particles_active_;

  // shouldn't be necessary, but keep around for debugging
  std::vector<Particle*> particles_;

  std::string name_;
};

} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
