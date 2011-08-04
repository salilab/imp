/**
 *  \file PbcBoxedMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_PBC_BOXED_MOVER_H
#define IMPMEMBRANE_PBC_BOXED_MOVER_H

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
class IMPMEMBRANEEXPORT PbcBoxedMover : public core::Mover
{
public:
  /** The particle is moved withing a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  PbcBoxedMover(Particle *p, Particles ps, Float max_tr,
                algebra::Vector3Ds centers,
                algebra::Transformation3Ds transformations);
  void reset_move();
  ParticlesTemp propose_move(Float f);
  IMP_OBJECT(PbcBoxedMover);
private:
  Float max_tr_;
  algebra::Transformation3Ds transformations_;
  algebra::Vector3Ds centers_;
  algebra::Vector3Ds oldcoords_;
  Particle *p_;
  Particles ps_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_PBC_BOXED_MOVER_H */
