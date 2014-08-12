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
#include <IMP/core/MonteCarloMover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>
//#include <IMP/core/mover_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

class IMPMEMBRANEEXPORT CellMover : public core::MonteCarloMover
{
public:
  CellMover(kernel::Model *m, kernel::ParticleIndex pi, Particles ps,
           Float max_translation);

  // IMP_MOVER(CellMover);
  IMP_OBJECT_METHODS(CellMover);

private:
  Float old_scale_;
  algebra::Vector3Ds oldcoords_;
  algebra::Transformation3Ds oldtrs_;
  kernel::ParticleIndex pi_;
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
