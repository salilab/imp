/**
 *  \file TransformedNuisance.cpp
 *  \brief Transform a bounded Nuisance for sampling in an unconstrained space.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/TransformedNuisance.h>
#include <IMP/core/JacobianAdjuster.h>
#include <boost/math/special_functions/log1p.hpp>

IMPISD_BEGIN_NAMESPACE

void TransformedNuisance::do_setup_particle(Model *m, ParticleIndex pi,
                                            double nuisance) {
  if (!TransformedNuisance::get_is_setup(m, pi)) {
    Nuisance::setup_particle(m, pi, nuisance);
    m->add_attribute(get_transformed_nuisance_key(), pi, 0.);

    core::UnivariateJacobian uj(1, 0, 0);
    IMP::Pointer<core::JacobianAdjuster> ja = core::get_jacobian_adjuster(m);
    ja->set_jacobian(get_transformed_nuisance_key(), pi, uj);
  }

  TransformedNuisance(m, pi).set_from_nuisance(nuisance);
  TransformedNuisance(m, pi).setup_score_state();
}

bool TransformedNuisance::get_is_setup(Model *m, ParticleIndex pi) {
  return (Nuisance::get_is_setup(m, pi) &&
          m->get_has_attribute(get_transformed_nuisance_key(), pi));
}

FloatKey TransformedNuisance::get_transformed_nuisance_key() {
  static FloatKey k("transformed nuisance");
  return k;
}

void TransformedNuisance::update_nuisance() const {
  double y, x, J, nlogJ, dnlogJ;
  y = get_transformed_nuisance();
  switch (get_transformation_type()) {
    case NONE: {
      x = y;
      J = 1;
      nlogJ = dnlogJ = 0;
      break;
    }
    case LOG_LOWER: {
      double expy = std::exp(y);
      x = get_lower() + expy;
      J = expy;
      nlogJ = -y;
      dnlogJ = -1;
      break;
    }
    case LOG_UPPER: {
      double expy = std::exp(y);
      x = get_upper() - expy;
      J = -expy;
      nlogJ = -y;
      dnlogJ = -1;
      break;
    }
    case LOGIT_LOWER_UPPER: {
      double expy, inv_logit, sign, a, range;
      a = get_lower();
      range = get_upper() - a;
      if (y > 0) {
        expy = std::exp(-y);
        inv_logit = 1 / (1 + expy);
        sign = -1;
        if (inv_logit == 1.) {
          inv_logit = 1 - 1e-15;
        }
      } else {
        expy = std::exp(y);
        inv_logit = 1 - 1 / (1 + expy);
        sign = 1;
        if (inv_logit == 0.) {
          inv_logit = 1e-15;
        }
      }
      x = a + range * inv_logit;
      J = range * (1 - inv_logit) * inv_logit;
      nlogJ = -std::log(range) - sign * y + 2 * boost::math::log1p(expy);
      dnlogJ = 2 * inv_logit - 1;
    }
  }

  get_particle()->set_value(get_nuisance_key(), x);

  core::UnivariateJacobian uj(J, nlogJ, dnlogJ);
  IMP::Pointer<core::JacobianAdjuster> ja =
      core::get_jacobian_adjuster(get_model());
  ja->set_jacobian(get_transformed_nuisance_key(), get_particle_index(), uj);
}

void TransformedNuisance::update_derivative() {
  DerivativeAccumulator accum = DerivativeAccumulator();
  add_to_transformed_nuisance_derivative(
      get_nuisance_derivative() * get_jacobian(), accum);
}

void TransformedNuisance::set_from_nuisance(Float x) {
  int trans_type = get_transformation_type();
  switch (trans_type) {
    case NONE:
      set_transformed_nuisance(x);
      break;
    case LOG_LOWER: {
      double a = get_lower();
      if (x <= a) {
        set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
      } else {
        set_transformed_nuisance(std::log(x - a));
      }
      break;
    }
    case LOG_UPPER: {
      double b = get_upper();
      if (x >= b) {
        set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
      } else {
        set_transformed_nuisance(std::log(b - x));
      }
      break;
    }
    case LOGIT_LOWER_UPPER: {
      double a = get_lower();
      double b = get_upper();
      if (x <= a) {
        set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
      } else if (x >= b) {
        set_transformed_nuisance(-std::log(std::numeric_limits<double>::min()));
      } else {
        set_transformed_nuisance(std::log(x - a) - std::log(b - x));
      }
    }
  }
  update_nuisance();
}

void TransformedNuisance::set_transformed_nuisance(Float y) {
  get_particle()->set_value(get_transformed_nuisance_key(), y);
}

double TransformedNuisance::get_jacobian() const {
  IMP::Pointer<core::JacobianAdjuster> ja =
      core::get_jacobian_adjuster(get_model());
  core::UnivariateJacobian uj = ja->access_jacobian(
    get_transformed_nuisance_key(), get_particle_index());
  return uj.get_jacobian();
}

double TransformedNuisance::get_score_adjustment() const {
  IMP::Pointer<core::JacobianAdjuster> ja =
      core::get_jacobian_adjuster(get_model());
  core::UnivariateJacobian uj = ja->access_jacobian(
    get_transformed_nuisance_key(), get_particle_index());
  return uj.get_score_adjustment();
}

double TransformedNuisance::get_gradient_adjustment() const {
  IMP::Pointer<core::JacobianAdjuster> ja =
      core::get_jacobian_adjuster(get_model());
  core::UnivariateJacobian uj = ja->access_jacobian(
    get_transformed_nuisance_key(), get_particle_index());
  return uj.get_gradient_adjustment();
}

void TransformedNuisance::set_lower(Float d) {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).set_lower(d);
  set_from_nuisance(n);
  //! We don't need the score state.
  remove_bounds();
}

void TransformedNuisance::set_lower(Particle *d) {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).set_lower(d);
  set_from_nuisance(n);
  //! We don't need the score state.
  remove_bounds();
}

void TransformedNuisance::remove_lower() {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).remove_lower();
  set_from_nuisance(n);
}

void TransformedNuisance::set_upper(Float d) {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).set_upper(d);
  set_from_nuisance(n);
  //! We don't need the score state.
  remove_bounds();
}

void TransformedNuisance::set_upper(Particle *d) {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).set_upper(d);
  set_from_nuisance(n);
  //! We don't need the score state.
  remove_bounds();
}

void TransformedNuisance::remove_upper() {
  update_nuisance();
  double n = get_nuisance();
  Nuisance(get_particle()).remove_upper();
  set_from_nuisance(n);
}

void TransformedNuisance::show(std::ostream &out) const {
  if (get_has_lower()) out << get_lower() << " < ";
  out << " TransformedNuisance = " << get_nuisance() << "("
      << get_transformed_nuisance() << ")";
  if (get_has_upper()) out << " < " << get_upper();
}

ObjectKey TransformedNuisance::get_ss_key() {
  static ObjectKey k("transformed_nuisance_ss");
  return k;
}

void TransformedNuisance::setup_score_state() {
  Pointer<Particle> p = get_particle();
  ObjectKey k(get_ss_key());
  if (p->has_attribute(k)) return;
  Pointer<TransformedNuisanceScoreState> ss(
      new TransformedNuisanceScoreState(p));
  ss->set_was_used(true);
  p->add_attribute(k, ss);
}

void TransformedNuisance::remove_score_state() {
  Pointer<Particle> p = get_particle();
  ObjectKey k(get_ss_key());
  if (!p->has_attribute(k)) return;
  Pointer<TransformedNuisanceScoreState> ss(
      dynamic_cast<TransformedNuisanceScoreState *>(p->get_value(k)));
  p->remove_attribute(k);
}

void TransformedNuisanceScoreState::do_before_evaluate() {
  TransformedNuisance nuis(p_);
  nuis.update_nuisance();
}

void TransformedNuisanceScoreState::do_after_evaluate(
    DerivativeAccumulator *da) {
  if (da) {
    TransformedNuisance nuis(p_);
    nuis.update_derivative();
  }
}

ModelObjectsTemp TransformedNuisanceScoreState::do_get_inputs() const {
  ModelObjectsTemp pt;
  pt.push_back(p_);
  ParticleIndexKey pu(TransformedNuisance(p_).get_upper_particle_key());
  if (p_->has_attribute(pu)) pt.push_back(p_->get_value(pu));
  ParticleIndexKey pd(TransformedNuisance(p_).get_lower_particle_key());
  if (p_->has_attribute(pd)) pt.push_back(p_->get_value(pd));
  return pt;
}

ModelObjectsTemp TransformedNuisanceScoreState::do_get_outputs() const {
  return ModelObjectsTemp(1, p_);
}

IMPISD_END_NAMESPACE
