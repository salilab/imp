/**
 *  \file BallMover.h    \brief A modifier which perturbs a discrete variable.
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

//! Modify a set of continuous variables.
/** The variables are perturbed within a ball of the 
    given radius.
    \ingroup mover
 */
class IMPDLLEXPORT BallMover :public MoverBase
{
public:
  BallMover(const Particles &pis, const FloatKeys &vars,
            Float max);
protected:
  void generate_move(float a);

private:
  Float max_step_;
};

} // namespace IMP

#endif  /* __IMP_BALL_MOVER_H */
