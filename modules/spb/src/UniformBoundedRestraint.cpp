/**
 *  \file spb/ UniformBoundedRestraint.h
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/PairContainer.h>
#include <IMP/constants.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/isd/Scale.h>
#include <IMP/spb/UniformBoundedRestraint.h>
#include <math.h>

IMPSPB_BEGIN_NAMESPACE

UniformBoundedRestraint::UniformBoundedRestraint(Particle *p, FloatKey fk,
                                                 Particle *a, Particle *b)
    : spb::ISDRestraint(p->get_model(), "UniformBoundedRestraint%1%") {
  p_ = p;
  fk_ = fk;
  a_ = a;
  b_ = b;
}

double UniformBoundedRestraint::get_probability() const {
  Float value = p_->get_value(fk_);
  // Float A = isd::Scale(a_).get_scale();
  // Float B = isd::Scale(b_).get_scale();

  Float A = isd::Nuisance(a_).get_nuisance();
  Float B = isd::Nuisance(b_).get_nuisance();
  Float prob = 0.;
  if (value >= A && value <= B) {
    prob = 1. / (B - A);
  }

  return prob;
}

double UniformBoundedRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  double score;

  double prob = get_probability();

  // check if probability is too low (e.g. equal to zero)
  // and assign its value to the smallest double
  if (prob <= std::numeric_limits<double>::epsilon()) {
    prob = std::numeric_limits<double>::epsilon();
  }

  score = -log(prob);

  if (accum) {
  }

  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
IMP::ModelObjectsTemp UniformBoundedRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  ret.push_back(p_);
  ret.push_back(a_);
  ret.push_back(b_);
  return ret;
}

IMPSPB_END_NAMESPACE
