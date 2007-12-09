/**
 *  \file CoordinateRestraint.h   \brief Absolute position restraint.
 *
 *  Optimize based on distance from an absolute position.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_COORDINATE_RESTRAINT_H
#define __IMP_COORDINATE_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../Restraint.h"

namespace IMP
{

//! Restrict particle position with respect to one or more coordinates
class IMPDLLEXPORT CoordinateRestraint : public Restraint
{
public:
  CoordinateRestraint(Model* model, Particle* p1, String type,
                      BasicScoreFuncParams* score_func_params);
  virtual ~CoordinateRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel");

protected:
  //! variables used to determine the distance
  FloatKey x_, y_, z_;
  //! type of coordinate position to use
  /** X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL, XYZ_SPHERE
   */
  std::string axis_;
  //! math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

} // namespace IMP

#endif /* __IMP_COORDINATE_RESTRAINT_H */
