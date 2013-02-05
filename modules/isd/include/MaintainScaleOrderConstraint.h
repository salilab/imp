/**
 *  \file IMP/isd/MaintainScaleOrderConstraint.h
 *  \brief Constrain scales to be ordered and positive.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_MAINTAIN_SCALE_ORDER_CONSTRAINT_H
#define IMPISD_MAINTAIN_SCALE_ORDER_CONSTRAINT_H

#include <IMP/isd/isd_config.h>
#include <IMP/Constraint.h>
#include <IMP/score_state_macros.h>
#include <IMP/Particle.h>
#include <IMP/isd/Scale.h>

/*
 * IMPKERNEL_BEGIN_NAMESPACE
// for swig
class TripletModifier;
IMPKERNEL_END_NAMESPACE
*/

IMPISD_BEGIN_NAMESPACE
//! Constrain scales to be ordered and positive.
/** The score state only acts before evaluation. Given a bunch of particles,
 * ensures that \f$0<\sigma_1\le\sigma_2\le\cdots\le\sigma_N\f$. It starts by
 * calling set_scale() on each scale, then gets their values, sorts them, and
 * sets them back.
 */
class IMPISDEXPORT MaintainScaleOrderConstraint : public Constraint
{
  Particles p_;
public:
  MaintainScaleOrderConstraint(const Particles& p,
                      std::string name="MaintainScaleOrderConstraint %1%");

  IMP_CONSTRAINT(MaintainScaleOrderConstraint);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_MAINTAIN_SCALE_ORDER_CONSTRAINT_H */
