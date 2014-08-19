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
//#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/core.h>
#include <IMP/algebra/Vector3D.h>

//#include <IMP/core/mover_macros.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Move a particle and keep it in a box
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPMEMBRANEEXPORT BoxedMover : public core::MonteCarloMover
{
public:
  /** The particle is moved withing a (hexagonal or cubic) box
      \param[in] p is the particle
      \param[in] max_tr is the maximum translation during a step
   */
 BoxedMover(Particle *p, Float max_tr, algebra::Vector3Ds centers);

protected:
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  void show(std::ostream &out) const;

  // IMP_MOVER(BoxedMover);

  IMP_OBJECT_METHODS(BoxedMover);

private:
  Float max_tr_;
  algebra::Vector3Ds centers_;
  algebra::Vector3D oldcoord_;
  IMP::base::PointerMember<kernel::Particle> p_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_BOXED_MOVER_H */
