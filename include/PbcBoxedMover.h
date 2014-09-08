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
#include <IMP/symmetry/BallMover.h>
#include <IMP/core.h>
#include <IMP/algebra/Vector3D.h>
//#include <IMP/core/mover_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT PbcBoxedMover: public symmetry::BallMover
{
public:
  /** The particle is moved withing a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  PbcBoxedMover(Particle *p, Particles ps, Float max_tr,
                algebra::Vector3Ds centers,
                algebra::Transformation3Ds transformations,
                Particle *px, Particle *py, Particle *pz);

protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(PbcBoxedMover);
  IMP_SHOWABLE(PbcBoxedMover);

private:
  /* All these are already in the base class
  //! Master particle
  IMP::base::PointerMember<kernel::Particle> p_;
  //! List of slave particles
  kernel::Particles ps_;
  //! Maximum translation
  Float max_tr_;
  //! List of centers of all (including primitive) cells
  algebra::Vector3Ds centers_;
  //! List of transformations from primitive to all (including primitive) cells
  algebra::Transformation3Ds tranformations_;
  // master particle old coordinates
  algebra::Vector3D oldcoord_;
  // slave particles old coordinates
  algebra::Vector3Ds oldcoords_;
  */

  // particles for scaling
  IMP::base::PointerMember<kernel::Particle> px_;
  IMP::base::PointerMember<kernel::Particle> py_;
  IMP::base::PointerMember<kernel::Particle> pz_;

  algebra::Vector3D get_vector(algebra::Vector3D center);
  algebra::Transformation3D get_transformation(algebra::Transformation3D trans);
};

IMPMEMBRANE_END_NAMESPACE

#endif /* IMPMEMBRANE_PBC_BOXED_MOVER_H */
