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
#include "Restraint.h"

namespace IMP
{

//! Restrict particle position with respect to one or more coordinates
class IMPDLLEXPORT CoordinateRestraint : public Restraint
{
public:
  CoordinateRestraint(Model& model, Particle* p1, String type,
                      BasicScoreFuncParams* score_func_params);
  virtual ~CoordinateRestraint();

  //! Calculate the score for this coordinate restraint.
  /** \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return Current score.
   */
  virtual Float evaluate(DerivativeAccumulator *accum);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out = std::cout) const;

  virtual std::string version(void) const {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  //! variables used to determine the distance
  FloatIndex x1_, y1_, z1_;
  //! type of coordinate position to use
  /** X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL, XYZ_SPHERE
   */
  std::string axis_;
  //! math form for this restraint (typically one of the harmonics)
  ScoreFunc* score_func_;
};

} // namespace IMP

#endif /* __IMP_COORDINATE_RESTRAINT_H */
