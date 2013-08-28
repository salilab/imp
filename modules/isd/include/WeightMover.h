/**
 *  \file isd/WeightMover.h
 *  \brief A mover that transform a rigid body
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_WEIGHT_MOVER_H
#define IMPISD_WEIGHT_MOVER_H

#include "isd_config.h"
#include <IMP/core/Mover.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/core/mover_macros.h>
#include <IMP/isd/Weight.h>

IMPISD_BEGIN_NAMESPACE

//! Modify the transformation of a rigid body
/** The transformation of a rigid body is moved in a ball of given
    size. The probability distribution is uniform over the ball.
    \see MonteCarlo
 */
class IMPISDEXPORT WeightMover : public core::MonteCarloMover
{
public:
  /** Mover for weight decorator
      \param[in] w particle
      \param[in] radius maximum radius of displacement
   */
  WeightMover(Particle *w, Float radius);

protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WeightMover);
private:
  Weight w_;
  algebra::VectorKD oldweights_;
  Float radius_;
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_WEIGHT_MOVER_H */
