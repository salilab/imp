/**
 *  \file IMP/spb/BoxedMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_BOXED_MOVER_H
#define IMPSPB_BOXED_MOVER_H

#include "spb_config.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/core.h>
#include <IMP/core/MonteCarloMover.h>

IMPSPB_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPSPBEXPORT BoxedMover : public core::MonteCarloMover {
 public:
  /** The particle is moved within a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
  BoxedMover(Particle *p, Float max_tr, algebra::Vector3Ds centers);

 protected:
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  // IMP_MOVER(BoxedMover);

  IMP_OBJECT_METHODS(BoxedMover);
  IMP_SHOWABLE(BoxedMover);

 private:
  Float max_tr_;
  algebra::Vector3Ds centers_;
  algebra::Vector3D oldcoord_;
  Particle *p_;
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_BOXED_MOVER_H */
