/**
 *  \file BallMover.h
 *  \brief A modifier which variables within a ball.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_BALL_MOVER_H
#define IMPCORE_BALL_MOVER_H

#include "config.h"
#include "MonteCarlo.h"
#include "MoverBase.h"

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables by perturbing them within a ball.
/** The variables are perturbed within a ball of the
    given radius.
    \see MonteCarlo
 */
class IMPCOREEXPORT BallMover :public MoverBase
{
public:
  /** The attributes are perturbed within a ball whose dimensionality is
      given by the number of attributes and radius by the given value.
      \param[in] sc The set of particles to perturb.
      \param[in] vars The variables to use (normally the keys for x,y,z)
      \param[in] radius The radius deviation to use.
   */
  BallMover(SingletonContainer *sc, const FloatKeys &vars,
            Float radius);
  /** */
  void set_radius(Float radius) {
    IMP_check(radius > 0, "The radius must be positive",
              ValueException);
    radius_=radius;
  }
  /** */
  Float get_radius() const {
    return radius_;
  }
protected:
  void generate_move(Float a);

private:
  Float radius_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BALL_MOVER_H */
