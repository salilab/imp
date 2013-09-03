/**
 *  \file isd/JeffreysRestraint.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/JeffreysRestraint.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

JeffreysRestraint::JeffreysRestraint(kernel::Particle *p):
    ISDRestraint("JeffreysRestraint_"+p->get_name()), p_(p) {
}

JeffreysRestraint::JeffreysRestraint(kernel::Model *m, kernel::Particle *p):
    ISDRestraint(m, "JeffreysRestraint_"+p->get_name()), p_(p) {
}

/* Apply the score if it's a scale decorator.
 */
double
JeffreysRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    Scale::decorate_particle(p_);
  }
  Scale sig(p_);
  double score;
  if (sig.get_scale() <= 0) {
      //std::cout << sig << std::endl;
      IMP_THROW("cannot use jeffreys prior on negative or zero scale",
              ModelException);
  }
  score=log(sig.get_scale());
  if (accum) {
    /* calculate derivative and add to 1st coordinate of sig */
    double deriv=1.0/sig.get_scale();
    sig.add_to_scale_derivative(deriv,*accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp JeffreysRestraint::do_get_inputs() const
{
  return kernel::ParticlesTemp(1,p_);
}

IMPISD_END_NAMESPACE
