/**
 *  \file isd/StudentTRestraint.cpp
 *  \brief A Student-t distribution restraint
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/StudentTRestraint.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/isd/Scale.h>

IMPISD_BEGIN_NAMESPACE

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     ParticleIndexAdaptor mu,
                                     ParticleIndexAdaptor sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      pmu_(mu),
      psigma_(sigma),
      pnu_(nu),
      isx_(true),
      ismu_(true),
      issigma_(true),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     double mu, ParticleIndexAdaptor sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      mu_(mu),
      psigma_(sigma),
      pnu_(nu),
      isx_(true),
      ismu_(false),
      issigma_(true),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     ParticleIndexAdaptor mu,
                                     double sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      pmu_(mu),
      sigma_(sigma),
      pnu_(nu),
      isx_(true),
      ismu_(true),
      issigma_(false),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     ParticleIndexAdaptor mu,
                                     ParticleIndexAdaptor sigma,
                                     double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      pmu_(mu),
      psigma_(sigma),
      nu_(nu),
      isx_(true),
      ismu_(true),
      issigma_(true),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     double mu, double sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      mu_(mu),
      sigma_(sigma),
      pnu_(nu),
      isx_(true),
      ismu_(false),
      issigma_(false),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     double mu, ParticleIndexAdaptor sigma,
                                     double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      mu_(mu),
      psigma_(sigma),
      nu_(nu),
      isx_(true),
      ismu_(false),
      issigma_(true),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     ParticleIndexAdaptor mu,
                                     double sigma, double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      pmu_(mu),
      sigma_(sigma),
      nu_(nu),
      isx_(true),
      ismu_(true),
      issigma_(false),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, ParticleIndexAdaptor x,
                                     double mu, double sigma, double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      px_(x),
      mu_(mu),
      sigma_(sigma),
      nu_(nu),
      isx_(true),
      ismu_(false),
      issigma_(false),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x,
                                     ParticleIndexAdaptor mu,
                                     ParticleIndexAdaptor sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      pmu_(mu),
      psigma_(sigma),
      pnu_(nu),
      isx_(false),
      ismu_(true),
      issigma_(true),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x, double mu,
                                     ParticleIndexAdaptor sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      mu_(mu),
      psigma_(sigma),
      pnu_(nu),
      isx_(false),
      ismu_(false),
      issigma_(true),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x,
                                     ParticleIndexAdaptor mu,
                                     double sigma,
                                     ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      pmu_(mu),
      sigma_(sigma),
      pnu_(nu),
      isx_(false),
      ismu_(true),
      issigma_(false),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x,
                                     ParticleIndexAdaptor mu,
                                     ParticleIndexAdaptor sigma,
                                     double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      pmu_(mu),
      psigma_(sigma),
      nu_(nu),
      isx_(false),
      ismu_(true),
      issigma_(true),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x, double mu,
                                     double sigma, ParticleIndexAdaptor nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      mu_(mu),
      sigma_(sigma),
      pnu_(nu),
      isx_(false),
      ismu_(false),
      issigma_(false),
      isnu_(true) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x, double mu,
                                     ParticleIndexAdaptor sigma,
                                     double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      mu_(mu),
      psigma_(sigma),
      nu_(nu),
      isx_(false),
      ismu_(false),
      issigma_(true),
      isnu_(false) {
  initialize();
}

StudentTRestraint::StudentTRestraint(Model *m, double x,
                                     ParticleIndexAdaptor mu,
                                     double sigma, double nu)
    : Restraint(m, "StudentTRestraint%1%"),
      x_(x),
      pmu_(mu),
      sigma_(sigma),
      nu_(nu),
      isx_(false),
      ismu_(true),
      issigma_(false),
      isnu_(false) {
  initialize();
}

void StudentTRestraint::initialize() {
  if (isx_) {
    IMP_USAGE_CHECK(Nuisance::get_is_setup(get_model(), px_),
                    "x particle should be a Nuisance!");
  }
  if (ismu_) {
    IMP_USAGE_CHECK(Nuisance::get_is_setup(get_model(), pmu_),
                    "mu particle should be a Nuisance!");
  }
  if (issigma_) {
    IMP_USAGE_CHECK(Scale::get_is_setup(get_model(), psigma_),
                    "sigma particle should be a Scale!");
  }
  if (isnu_) {
    IMP_USAGE_CHECK(Scale::get_is_setup(get_model(), pnu_),
                    "nu particle should be a Scale!");
  }
  static double logJX = 0.;  // log derivative of identity (1.)
  double x = get_x();
  studentt_ = new FStudentT(x, x * x, 1, logJX, get_mu(), get_sigma(),
                            get_nu());
}

void StudentTRestraint::update_distribution() {
  if (isx_) {
    double x = Nuisance(get_model(), px_).get_nuisance();
    studentt_->set_sumFX(x);
    studentt_->set_sumFX2(x * x);
  }
  if (ismu_) {
    studentt_->set_FM(Nuisance(get_model(), pmu_).get_nuisance());
  }
  if (issigma_) {
    studentt_->set_sigma(Scale(get_model(), psigma_).get_scale());
  }
  if (isnu_) {
    studentt_->set_nu(Scale(get_model(), pnu_).get_scale());
  }
}

double StudentTRestraint::get_x() const {
  return (isx_) ? Nuisance(get_model(), px_).get_nuisance() : x_;
}

double StudentTRestraint::get_mu() const {
  return (ismu_) ? Nuisance(get_model(), pmu_).get_nuisance() : mu_;
}

double StudentTRestraint::get_sigma() const {
  return (issigma_) ? Scale(get_model(), psigma_).get_scale() : sigma_;
}

double StudentTRestraint::get_nu() const {
  return (isnu_) ? Scale(get_model(), pnu_).get_scale() : nu_;
}

double StudentTRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  const_cast<StudentTRestraint *>(this)->update_distribution();
  double score = studentt_->evaluate();
  studentt_->set_was_used(true);

  if (accum) {
    if (isx_ || ismu_) {
      double DFM = studentt_->evaluate_derivative_FM();
      if (isx_) {
        Nuisance(get_model(), px_).add_to_nuisance_derivative(-DFM, *accum);
      }
      if (ismu_) {
        Nuisance(get_model(), pmu_).add_to_nuisance_derivative(DFM, *accum);
      }
    }
    if (issigma_) {
      Scale(get_model(), psigma_).add_to_scale_derivative(
        studentt_->evaluate_derivative_sigma(), *accum);
    }
    if (isnu_) {
      Scale(get_model(), pnu_).add_to_scale_derivative(
        studentt_->evaluate_derivative_nu(), *accum);
    }
  }
  return score;
}

ModelObjectsTemp StudentTRestraint::do_get_inputs() const {
  ParticlesTemp ret;
  if (isx_) ret.push_back(get_model()->get_particle(px_));
  if (ismu_) ret.push_back(get_model()->get_particle(pmu_));
  if (issigma_) ret.push_back(get_model()->get_particle(psigma_));
  if (isnu_) ret.push_back(get_model()->get_particle(pnu_));
  return ret;
}

IMPISD_END_NAMESPACE
