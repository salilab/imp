/**
 *  \file CellMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_CELL_MOVER_H
#define IMPMEMBRANE_CELL_MOVER_H

#include "membrane_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/mover_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT CellMover : public core::Mover
{
public:
  /** The rigid body is rotated and translated to move
      \param[in] d   the rigid body decorator
      \param[in] max_translation maximum translation during a step
      \param[in] max_rotation maximum rotation angle in radians
   */
  CellMover(Particle *p, Particles ps, Float max_translation);

  IMP_MOVER(CellMover);
private:
  Float old_scale_;
  algebra::Vector3Ds oldcoords_;
  algebra::Transformation3Ds oldtrs_;
  Pointer<Particle> p_;
  Particles ps_;
  Float max_translation_;
  Particles ps_norb_;
  std::vector<core::RigidBody> rbs_;

  Particles         get_particles(Particles ps);
  std::vector<core::RigidBody> get_rigid_bodies(Particles ps);
  algebra::Vector3D get_transformed(Float cf, algebra::Vector3D oc);
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_CELL_MOVER_H */
