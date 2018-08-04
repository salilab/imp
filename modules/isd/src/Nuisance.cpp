/**
 *  \file Nuisance.cpp
 *  \brief defines a one-dimensional nuisance parameter (sigma, gamma, ...)
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

void Nuisance::do_setup_particle(Model *m, ParticleIndex pi,
                                 double nuisance) {
  if (!Nuisance::get_is_setup(m, pi)) {
    m->add_attribute(get_nuisance_key(), pi, nuisance);
  } else {
    Nuisance(m, pi).set_nuisance(nuisance);
  }
}

FloatKey Nuisance::get_transformed_nuisance_key() {
  static FloatKey k("nuisance");
  return k;
}

void Nuisance::set_nuisance(Float x) {
  int trans_type = get_transformation_type();
  switch(trans_type) {
    case NONE:
      set_transformed_nuisance(x);
      break;
    case LOG_LOWER:
      {
        double lo = get_lower();
        if (x <= lo) {
          set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
        } else {
          set_transformed_nuisance(std::log(x - lo));
        }
        break;
      }
    case LOG_UPPER: {
        double up = get_upper();
        if (x >= up) {
          set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
        } else {
          set_transformed_nuisance(std::log(up - x));
        }
        break;
    }
    case LOGIT_LOWER_UPPER: {
      double lo = get_lower();
      double up = get_upper();
      if (x <= lo) {
        set_transformed_nuisance(std::log(std::numeric_limits<double>::min()));
      } else if (x >= up) {
        set_transformed_nuisance(std::log(std::numeric_limits<double>::max()));
      } else {
        set_transformed_nuisance(std::log((x - get_lower()) / (get_upper() - x)));
      }
    }
  }
}

Float Nuisance::get_nuisance() const {
  double y = get_transformed_nuisance();
  switch(get_transformation_type()) {
    case NONE: return y;
    case LOG_LOWER: return get_lower() + std::exp(y);
    case LOG_UPPER: return get_upper() - std::exp(y);
    case LOGIT_LOWER_UPPER:
      double lo = get_lower();
      return lo + (get_upper() - lo) / (1 + std::exp(-y));
  }
}

void Nuisance::set_transformed_nuisance(Float y) {
  Particle *p = get_particle();
  p->set_value(get_nuisance_key(), y);
}

Float Nuisance::get_nuisance_derivative() const {
  return (
    get_transformed_nuisance_derivative()
    - get_derivative_of_negative_log_absolute_jacobian_of_transformation()
    ) * get_jacobian_of_transformation();
}

void Nuisance::add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
  add_to_transformed_nuisance_derivative(d / get_jacobian_of_transformation(), accum);
}

double Nuisance::get_jacobian_of_transformation() const {
  if (get_transformation_type() == LOG_UPPER) {
    return -std::exp(get_transformed_nuisance());
  } else {
    return std::exp(-get_negative_log_absolute_jacobian_of_transformation());
  }
}

double Nuisance::get_negative_log_absolute_jacobian_of_transformation() const {
  double y = get_transformed_nuisance();
  switch(get_transformation_type()) {
    case NONE: return 0;
    case LOG_LOWER: return -y;
    case LOG_UPPER: return -y;
    case LOGIT_LOWER_UPPER:
      return y + 2 * std::log1p(std::exp(-y))
               - std::log(get_upper() - get_lower());
  }
}

double Nuisance::get_derivative_of_negative_log_absolute_jacobian_of_transformation() const {
  switch(get_transformation_type()) {
    case NONE: return 0;
    case LOG_LOWER: return -1;
    case LOG_UPPER: return -1;
    case LOGIT_LOWER_UPPER:
      double expy = std::exp(-get_transformed_nuisance());
      return 1 - 2 * expy / (1 + expy);
  }
}

bool Nuisance::get_has_lower() const {
  Pointer<Particle> p = get_particle();
  return p->has_attribute(get_lower_key()) ||
         p->has_attribute(get_lower_particle_key());
}
Float Nuisance::get_lower() const {
  Particle *p = get_particle();
  FloatKey lk(get_lower_key());
  ParticleKey lp(get_lower_particle_key());
  bool hasfloat = p->has_attribute(lk);
  bool hasparticle = p->has_attribute(lp);
  if (hasfloat) {
    double floatval = p->get_value(lk);
    if (hasparticle) {
      Particle *d = p->get_value(lp);
      double particleval = d->get_value(get_nuisance_key());
      if (floatval > particleval) {
        return floatval;
      } else {
        return particleval;
      }
    } else {
      return floatval;
    }
  } else if (hasparticle) {
    Particle *d = p->get_value(lp);
    return d->get_value(get_nuisance_key());
  } else {
    return -std::numeric_limits<double>::infinity();
  }
}
FloatKey Nuisance::get_lower_key() {
  static FloatKey k("lower");
  return k;
}
ParticleIndexKey Nuisance::get_lower_particle_key() {
  static ParticleIndexKey k("lower");
  return k;
}
void Nuisance::set_lower(Float d) {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  FloatKey k(get_lower_key());
  if (!p->has_attribute(k)) p->add_attribute(k, d);
  p->set_value(k, d);
  setup_score_state();
  set_nuisance(x);
}
void Nuisance::set_lower(Particle *d) {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  ParticleKey k(get_lower_particle_key());
  if (!p->has_attribute(k)) p->add_attribute(k, d);
  p->set_value(k, d);
  setup_score_state();
  set_nuisance(x);
}
void Nuisance::remove_lower() {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  FloatKey k(get_lower_key());
  if (p->has_attribute(k)) p->remove_attribute(k);
  FloatKey kp(get_lower_key());
  if (p->has_attribute(kp)) p->remove_attribute(kp);
  remove_score_state();
  set_nuisance(x);
}

bool Nuisance::get_has_upper() const {
  Pointer<Particle> p = get_particle();
  return p->has_attribute(get_upper_key()) ||
         p->has_attribute(get_upper_particle_key());
}
Float Nuisance::get_upper() const {
  Particle *p = get_particle();
  FloatKey ku(get_upper_key());
  ParticleKey kp(get_upper_particle_key());
  bool hasfloat = p->has_attribute(ku);
  bool hasparticle = p->has_attribute(kp);
  if (hasfloat) {
    double floatval = p->get_value(ku);
    if (hasparticle) {
      Particle *d = p->get_value(kp);
      double particleval = d->get_value(get_nuisance_key());
      if (floatval < particleval) {
        return floatval;
      } else {
        return particleval;
      }
    } else {
      return floatval;
    }
  } else if (hasparticle) {
    Particle *d = p->get_value(kp);
    return d->get_value(get_nuisance_key());
  } else {
    return std::numeric_limits<double>::infinity();
  }
}
FloatKey Nuisance::get_upper_key() {
  static FloatKey k("upper");
  return k;
}
ParticleIndexKey Nuisance::get_upper_particle_key() {
  static ParticleIndexKey k("upper");
  return k;
}
void Nuisance::set_upper(Float d) {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  FloatKey k(get_upper_key());
  if (!p->has_attribute(k)) p->add_attribute(k, d);
  p->set_value(k, d);
  setup_score_state();
  set_nuisance(x);
}
void Nuisance::set_upper(Particle *d) {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  ParticleKey k(get_upper_particle_key());
  if (!p->has_attribute(k)) p->add_attribute(k, d);
  p->set_value(k, d);
  setup_score_state();
  set_nuisance(x);
}
void Nuisance::remove_upper() {
  double x = get_nuisance();
  Pointer<Particle> p = get_particle();
  FloatKey k(get_upper_key());
  if (p->has_attribute(k)) p->remove_attribute(k);
  ParticleKey kp(get_upper_particle_key());
  if (p->has_attribute(kp)) p->remove_attribute(kp);
  remove_score_state();
  set_nuisance(x);
}

void Nuisance::show(std::ostream &out) const {
  if (get_has_lower()) out << get_lower() << " < ";
  out << " Nuisance = " << get_nuisance();
  if (get_has_upper()) out << " < " << get_upper();
}

ObjectKey Nuisance::get_ss_key() {
  static ObjectKey k("nuisance_ss");
  return k;
}
void Nuisance::setup_score_state() {
  Pointer<Particle> p = get_particle();
  ObjectKey k(get_ss_key());
  if (p->has_attribute(k)) return;
  Pointer<NuisanceScoreState> ss(new NuisanceScoreState(p));
  ss->set_was_used(true);
  p->add_attribute(k, ss);
}
void Nuisance::remove_score_state() {
  Pointer<Particle> p = get_particle();
  ObjectKey k(get_ss_key());
  if (!p->has_attribute(k)) return;
  Pointer<NuisanceScoreState> ss(
      dynamic_cast<NuisanceScoreState *>(p->get_value(k)));
  p->remove_attribute(k);
}

void NuisanceScoreState::do_before_evaluate() {
}

void NuisanceScoreState::do_after_evaluate(DerivativeAccumulator *da) {
  if (da) {
    Nuisance nuis(p_);
    nuis.add_to_transformed_nuisance_derivative(
      nuis.get_derivative_of_negative_log_absolute_jacobian_of_transformation(),
      *da);
  }
}

ModelObjectsTemp NuisanceScoreState::do_get_inputs() const {
  ModelObjectsTemp pt;
  pt.push_back(p_);
  ParticleIndexKey pu(Nuisance(p_).get_upper_particle_key());
  if (p_->has_attribute(pu)) pt.push_back(p_->get_value(pu));
  ParticleIndexKey pd(Nuisance(p_).get_lower_particle_key());
  if (p_->has_attribute(pd)) pt.push_back(p_->get_value(pd));
  return pt;
}

ModelObjectsTemp NuisanceScoreState::do_get_outputs() const {
  return ModelObjectsTemp(1, p_);
}

IMPISD_END_NAMESPACE
