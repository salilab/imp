/**
 *  \file isd/SlidingPriorRestraint.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/SlidingPriorRestraint.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

SlidingPriorRestraint::SlidingPriorRestraint(Particle *p, double qmin,
                                             double qmax, double sq)
  : p_(p), qmin_(qmin), qmax_(qmax) {
  sq_ = sq*(qmax_-qmin_);
}


/* Apply the score if it's a scale decorator.
 */
double
SlidingPriorRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    Scale::decorate_particle(p_);
  }
  Scale q0(p_);
  double score;
  if (q0.get_scale() <= 0) {
      IMP_THROW("cannot use jeffreys prior on negative or zero scale",
              ModelException);
  }
  double qval = q0.get_scale();
  score=-0.5*square((qval - qmin_)/sq_) + log(sq_);
  if (accum) {
    /* calculate derivative and add to 1st coordinate of sig */
    double deriv=-(qval-qmin_)/square(sq_);
    q0.add_to_scale_derivative(deriv,*accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp SlidingPriorRestraint::do_get_inputs() const
{
  return ParticlesTemp(1,p_);
}

IMPISD_END_NAMESPACE
