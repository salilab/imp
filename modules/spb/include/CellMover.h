/**
 *  \file IMP/spb/CellMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_CELL_MOVER_H
#define IMPSPB_CELL_MOVER_H

#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/spb/spb_config.h>

IMPSPB_BEGIN_NAMESPACE

//! Apply a mover that moves particles inside the unit cell.
/** The mover takes into account the fact that the unit cell size
    changes from one step to the next.
 */
class IMPSPBEXPORT CellMover : public core::MonteCarloMover {
 public:
  CellMover(Particle *p, Particles ps, Float max_translation);

  // IMP_MOVER(CellMover);
 protected:
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(CellMover);
  IMP_SHOWABLE(CellMover);

 private:
  Float old_scale_;
  algebra::Vector3Ds oldcoords_;
  algebra::Transformation3Ds oldtrs_;
  IMP::PointerMember<IMP::Particle> p_;
  Particles ps_;
  Float max_translation_;
  Particles ps_norb_;
  std::vector<core::RigidBody> rbs_;

  Particles get_particles(Particles ps);
  std::vector<core::RigidBody> get_rigid_bodies(Particles ps);
  algebra::Vector3D get_transformed(Float cf, algebra::Vector3D oc);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_CELL_MOVER_H */
