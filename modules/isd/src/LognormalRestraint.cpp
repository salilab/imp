/**
 *  \file isd/LognormalRestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/LognormalRestraint.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/isd/Scale.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

LognormalRestraint::LognormalRestraint(Particle *x,
                                       Particle *mu,
                                       Particle *sigma)
    : Restraint(sigma->get_model(), "LognormalRestraint%1%"),
      px_(x),
      pmu_(mu),
      psigma_(sigma),
      isx_(true),
      ismu_(true),
      issigma_(true) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(double x, Particle *mu,
                                       Particle *sigma)
    : Restraint(sigma->get_model(), "LognormalRestraint%1%"),
      x_(x),
      pmu_(mu),
      psigma_(sigma),
      isx_(false),
      ismu_(true),
      issigma_(true) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(Particle *x, double mu,
                                       Particle *sigma)
    : Restraint(sigma->get_model(), "LognormalRestraint%1%"),
      px_(x),
      mu_(mu),
      psigma_(sigma),
      isx_(true),
      ismu_(false),
      issigma_(true) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(Particle *x,
                                       Particle *mu, double sigma)
    : Restraint(x->get_model(), "LognormalRestraint%1%"),
      px_(x),
      pmu_(mu),
      sigma_(sigma),
      isx_(true),
      ismu_(true),
      issigma_(false) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(Particle *x, double mu,
                                       double sigma)
    : Restraint(x->get_model(), "LognormalRestraint%1%"),
      px_(x),
      mu_(mu),
      sigma_(sigma),
      isx_(true),
      ismu_(false),
      issigma_(false) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(double x, double mu,
                                       Particle *sigma)
    : Restraint(sigma->get_model(), "LognormalRestraint%1%"),
      x_(x),
      mu_(mu),
      psigma_(sigma),
      isx_(false),
      ismu_(false),
      issigma_(true) {
  check_particles();
  create_normal();
}

LognormalRestraint::LognormalRestraint(double x, Particle *mu,
                                       double sigma)
    : Restraint(mu->get_model(), "LognormalRestraint%1%"),
      x_(x),
      pmu_(mu),
      sigma_(sigma),
      isx_(false),
      ismu_(true),
      issigma_(false) {
  check_particles();
  create_normal();
}

void LognormalRestraint::check_particles() {
  IMP_IF_CHECK(USAGE) {
    if (isx_) {
      IMP_USAGE_CHECK(Nuisance::get_is_setup(px_),
                      "x particle should be a Nuisance!");
    }
    if (ismu_) {
      IMP_USAGE_CHECK(Nuisance::get_is_setup(pmu_),
                      "mu particle should be a Nuisance!");
    }
    if (issigma_) {
      IMP_USAGE_CHECK(Scale::get_is_setup(psigma_),
                      "sigma particle should be a Scale!");
    }
  }
}

void LognormalRestraint::create_normal() {
  double x, mu, sigma;
  x = (isx_) ? Nuisance(px_).get_nuisance() : x_;
  mu = (ismu_) ? Nuisance(pmu_).get_nuisance() : mu_;
  sigma = (issigma_) ? Scale(psigma_).get_scale() : sigma_;
  IMP_NEW(FNormal, normal, (x, 1. / x, mu, sigma));
  normal_ = normal.release();
}

/* Apply the restraint to two atoms, two Scales, one experimental value.
 */
double LognormalRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  double x, mu, sigma;
  x = (isx_) ? Nuisance(px_).get_nuisance() : x_;
  mu = (ismu_) ? Nuisance(pmu_).get_nuisance() : mu_;
  sigma = (issigma_) ? Scale(psigma_).get_scale() : sigma_;
  /* compute all arguments to FNormal */
  double lx = log(x);
  double lmu = log(mu);
  normal_->set_FA(lx);
  normal_->set_JA(1. / x);
  normal_->set_FM(lmu);
  normal_->set_sigma(sigma);    
  /* get score */
  double score = normal_->evaluate();
  const_cast<LognormalRestraint *>(this)->set_chi(lx - lmu);

  if (accum) {
    if (isx_ || ismu_) {
      // d(score)/dM = d(score)/dF(M) * dF(M)/dM
      double DFM = normal_->evaluate_derivative_FM();
      if (isx_) Nuisance(px_).add_to_nuisance_derivative((1 - DFM) / x, *accum);
      if (ismu_) Nuisance(pmu_).add_to_nuisance_derivative(DFM / mu, *accum);
    }
    if (issigma_)
      Scale(psigma_)
          .add_to_scale_derivative(normal_->evaluate_derivative_sigma(), *accum);
  }
  return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp LognormalRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  if (isx_) ret.push_back(px_);
  if (ismu_) ret.push_back(pmu_);
  if (issigma_) ret.push_back(psigma_);
  return ret;
}

IMPISD_END_NAMESPACE
