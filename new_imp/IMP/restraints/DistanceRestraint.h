/*
 *  \file DistanceRestraint.h     Distance restraint between two particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_DISTANCE_RESTRAINT_H
#define __IMP_DISTANCE_RESTRAINT_H

#include <vector>
#include <iostream>

#include "../IMP_config.h"
#include "../ModelData.h"
#include "../ScoreFunc.h"

namespace imp
{

//! Distance restraint between two particles
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

#endif  /* __IMP_DISTANCE_RESTRAINT_H */
