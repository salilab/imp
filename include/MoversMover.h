/**
 *  \file MoversMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MOVERS_MOVER_H
#define IMPMEMBRANE_MOVERS_MOVER_H

#include "membrane_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT MoversMover : public core::Mover
{
public:
  /** The particle is moved withing a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  MoversMover(core::Movers mvs);
  void reset_move();
  void propose_move(Float f);
  IMP_OBJECT(MoversMover);
private:
  core::Movers mvs_;
  int imov_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MOVERS_MOVER_H */
