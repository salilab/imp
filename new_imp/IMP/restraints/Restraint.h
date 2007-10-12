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

namespace imp
{

// Abstract class for representing restraints
class IMPDLLEXPORT Restraint
{
public:
  Restraint();
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
};

// Distance restraint between two particles
class IMPDLLEXPORT DistanceRestraint : public Restraint
{
public:
  // particles must be at least this far apart to calculate the
  // ... distance restraint. Force it otherwise.
  static const Float MIN_DISTANCE;

  DistanceRestraint(Model& model,
               Particle* p1,
               Particle* p2,
               BasicScoreFuncParams* score_func_params);
  DistanceRestraint(Model& model,
               Particle* p1,
               Particle* p2,
               const std::string attr_name,
               BasicScoreFuncParams* score_func_params);
  virtual ~DistanceRestraint();

  virtual Float evaluate(bool calc_deriv);

  // status
  virtual void show (std::ostream& out = std::cout) const;
  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  void set_up(Model& model,
              Particle* p1,
              Particle* p2,
              BasicScoreFuncParams* score_func_params);

  // variables used to determine the distance
  FloatIndex x1_, y1_, z1_, x2_, y2_, z2_;

  // variables used to calculate the math form
  Float mean_, sd_;
  // math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

} // namespace imp

#endif  /* __IMP_RESTRAINT_H */
