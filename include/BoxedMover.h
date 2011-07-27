/**
 *  \file BoxedMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_BOXED_MOVER_H
#define IMPMEMBRANE_BOXED_MOVER_H

#include "membrane_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/core.h>
#include <IMP/algebra/Vector3D.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT BoxedMover : public core::Mover
{
public:
  /** The particle is moved withing a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  BoxedMover(Particle *p, Float max_tr,
             Float side, Float thick, algebra::VectorD<3> center,
             std::string cell_type);
  void reset_move();
  void propose_move(Float f);
  IMP_OBJECT(BoxedMover);
private:
  Float max_tr_;
  Float side_;
  Float thick_;
  algebra::VectorD<3> center_, oldcoord_;
  std::string cell_type_;
  Particle *p_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_BOXED_MOVER_H */
