/**
 *  \file BallMover.h
 *  \brief A modifier which variables within a ball.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INCREMENTAL_BALL_MOVER_H
#define IMPCORE_INCREMENTAL_BALL_MOVER_H

#include "config.h"
#include "MonteCarlo.h"
#include "MoverBase.h"
#include "macros.h"

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables by perturbing them within a ball.
/** This modifier is designed to be used when doing incremental updates. See
    BallMover for a variant that moves all at the same time.
    \see MonteCarlo
    \untested{IncrementalBallMover}
    \unstable{IncrmenetalBallMover}
 */
class IMPCOREEXPORT IncrementalBallMover :public Mover
{
public:
  /** The attributes are perturbed within a ball whose dimensionality is
      given by the number of attributes and radius by the given value.
      \param[in] sc The set of particles to perturb.
      \param[in] n The number of points to move at the same time.
      \param[in] vars The variables to use (normally the keys for x,y,z)
      \param[in] radius The radius deviation to use.
   */
  IncrementalBallMover(SingletonContainer *sc,
                       unsigned int n,
                       Float radius);
  IMP_MOVER(IncrementalBallMover, internal::version_info)
private:
  Pointer<SingletonContainer> sc_;
  unsigned int n_;
  Float radius_;
  Particles moved_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_INCREMENTAL_BALL_MOVER_H */
