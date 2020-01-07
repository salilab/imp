/**
 *  \file IMP/spb/PbcBoxedMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_PBC_BOXED_MOVER_H
#define IMPSPB_PBC_BOXED_MOVER_H

#include <IMP/algebra/Vector3D.h>
#include <IMP/core.h>
#include <IMP/core/MonteCarlo.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPSPBEXPORT PbcBoxedMover : public core::MonteCarloMover {
 public:
  /** The particle is moved within a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  PbcBoxedMover(Particle *p, Particles ps, Float max_tr,
                algebra::Vector3Ds centers,
                algebra::Transformation3Ds transformations, Particle *px,
                Particle *py, Particle *pz);

 protected:
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(PbcBoxedMover);
  IMP_SHOWABLE(PbcBoxedMover);

 private:
  Float max_tr_;
  algebra::Transformation3Ds transformations_;
  algebra::Vector3Ds centers_;
  algebra::Vector3Ds oldcoords_;
  IMP::PointerMember<IMP::Particle> p_;
  IMP::Particles ps_;
  IMP::PointerMember<IMP::Particle> px_;
  IMP::PointerMember<IMP::Particle> py_;
  IMP::PointerMember<IMP::Particle> pz_;

  algebra::Vector3D get_vector(algebra::Vector3D center);
  algebra::Transformation3D get_transformation(algebra::Transformation3D trans);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_PBC_BOXED_MOVER_H */
