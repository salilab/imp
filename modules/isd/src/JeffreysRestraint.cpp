/**
 *  \file isd/JeffreysRestraint.cpp
 *  \brief Restrain a scale particle with log(scale)
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Scale.h>
#include <IMP/isd/JeffreysRestraint.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

JeffreysRestraint::JeffreysRestraint(Model *m, Particle *p)
    : Restraint(m, "JeffreysRestraint_" + p->get_name()), p_(p) {}

/* Apply the score if it's a scale decorator.
 */
double JeffreysRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  Scale sig(p_);
  double score;
  if (sig.get_scale() <= 0) {
    // std::cout << sig << std::endl;
    IMP_THROW("cannot use jeffreys prior on negative or zero scale",
              ModelException);
  }
  score = log(sig.get_scale());
  if (accum) {
    /* calculate derivative and add to 1st coordinate of sig */
    double deriv = 1.0 / sig.get_scale();
    sig.add_to_scale_derivative(deriv, *accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
kernel::ModelObjectsTemp JeffreysRestraint::do_get_inputs() const {
  return ParticlesTemp(1, p_);
}

IMPISD_END_NAMESPACE
