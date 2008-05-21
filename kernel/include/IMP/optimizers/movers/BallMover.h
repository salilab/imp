/**
 *  \file BallMover.h
 *  \brief A modifier which variables within a ball.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BALL_MOVER_H
#define __IMP_BALL_MOVER_H

#include "../../IMP_config.h"
#include "../MonteCarlo.h"
#include "../MoverBase.h"

namespace IMP
{

//! Modify a set of continuous variables by perturbing them within a ball.
/** The variables are perturbed within a ball of the 
    given radius.
    \ingroup mover
 */
class IMPDLLEXPORT BallMover :public MoverBase
{
public:
  /** The attributes are perturbed within a pall whose dimensionality is 
      given by the number of attributes and radius by the given value.
     \param[in] vars The variables to use (normally the keys for x,y,z)
     \param[in] radius The radius deviation to use.
     \param[in] ps The particles to perturb.
   */
  BallMover(const FloatKeys &vars,
            Float radius, const Particles &ps);
  //!
  void set_radius(Float radius) {
    IMP_check(radius > 0, "The radius must be positive",
              ValueException("Negative radius"));
    radius_=radius;
  }
  //!
  Float get_radius() const {
    return radius_;
  }
protected:
  /** \internal */
  void generate_move(float a);

private:
  Float radius_;
};

} // namespace IMP

#endif  /* __IMP_BALL_MOVER_H */
