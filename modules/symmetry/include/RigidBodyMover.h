/**
 *  \file IMP/symmetry/RigidBodyMover.h
 *  \brief A mover that keeps a rigid body in a box
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSYMMETRY_RIGID_BODY_MOVER_H
#define IMPSYMMETRY_RIGID_BODY_MOVER_H

#include <IMP/symmetry/symmetry_config.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/SingletonContainer.h>
#include <IMP/core/rigid_bodies.h>

IMPSYMMETRY_BEGIN_NAMESPACE

//! Move a rigid body and keep it in the primitive cell of a periodic lattice
class IMPSYMMETRYEXPORT RigidBodyMover : public core::MonteCarloMover {
 public:
  /** The rigid body is rotated and translated to move
      \param[in] d is the master rigid body decorator
      \param[in] ps are the slave particles
      \param[in] max_tr is the maximum translation during a step
      \param[in] max_ang is the maximum rotation angle in radians
      \param[in] ctrs is a list of all cell centers
      \param[in] trs is the list of transformations from primitive to all cells
   */
  RigidBodyMover(core::RigidBody d, Particles ps, Float max_tr,
                 Float max_ang, algebra::Vector3Ds ctrs,
                 algebra::Transformation3Ds trs);

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodyMover);

 private:
  core::RigidBody d_;
  Particles ps_;
  Float max_tr_;
  Float max_ang_;
  algebra::Vector3Ds ctrs_;
  algebra::Transformation3Ds trs_;
  Particles ps_norb_;
  std::vector<core::RigidBody> rbs_;
  algebra::Transformation3D oldtr_;
  algebra::Vector3Ds oldcoords_;
  algebra::Transformation3Ds oldtrs_;

  Particles get_particles(Particles ps);
  std::vector<core::RigidBody> get_rigid_bodies(Particles ps);
};

IMPSYMMETRY_END_NAMESPACE

#endif /* IMPSYMMETRY_RIGID_BODY_MOVER_H */
