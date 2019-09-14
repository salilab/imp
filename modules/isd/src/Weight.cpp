/**
 *  \file isd/Weight.cpp
 *  \brief Add weights constrained to the unit simplex to a particle.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Weight.h>
#include <IMP/exception.h>
#include <limits.h>
#include <sstream>

IMPISD_BEGIN_NAMESPACE

void Weight::do_setup_particle(Model *m, ParticleIndex pi) {
  IMPISD_DEPRECATED_METHOD_DEF(
    2.12,
    "Use do_setup_particle(m, pi, nweights) or do_setup_particle(m, pi, w) instead."
  );

  m->add_attribute(get_number_of_weights_key(), pi, 0);

  add_constraint(m, pi);
}

void Weight::do_setup_particle(Model *m, ParticleIndex pi, Int nweights) {
  IMP_USAGE_CHECK(nweights > 0, "Number of weights must be greater than zero.");
  m->add_attribute(get_number_of_weights_key(), pi, nweights);

  Float wi = 1.0 / static_cast<Float>(nweights);
  for (int i = 0; i < nweights; ++i)
    m->add_attribute(get_weight_key(i), pi, wi);

  add_constraint(m, pi);
}

void Weight::do_setup_particle(Model *m, ParticleIndex pi,
                               const algebra::VectorKD &w) {
  Int nweights = w.get_dimension();
  IMP_USAGE_CHECK(nweights > 0, "Number of weights must be greater than zero.");
  m->add_attribute(get_number_of_weights_key(), pi, nweights);

  m->add_attribute(get_weight_key(0), pi, w[0]);
  for (int i = 1; i < nweights; ++i)
    m->add_attribute(get_weight_key(i), pi, w[i]);

  add_constraint(m, pi);

  Weight pw(m, pi);
  pw.set_weights(pw.get_weights());
}

void Weight::add_constraint(Model *m, ParticleIndex pi) {
  ObjectKey k(get_constraint_key());
  Pointer<WeightSimplexConstraint> c(new WeightSimplexConstraint(
    m->get_particle(pi)));
  c->set_was_used(true);
  m->get_particle(pi)->add_attribute(k, c);
  m->add_score_state(c);
}

bool Weight::get_is_setup(Model *m, ParticleIndex pi) {
  if (!m->get_has_attribute(get_number_of_weights_key(), pi)) return false;
  if (!m->get_has_attribute(get_constraint_key(), pi)) return false;
  Int nweights = m->get_attribute(get_number_of_weights_key(), pi);
  for (unsigned int i = 0; i < nweights; ++i)
    if (!m->get_has_attribute(get_weight_key(i), pi)) return false;
  return true;
}

IntKey Weight::get_nstates_key() {
  IMPISD_DEPRECATED_METHOD_DEF(
    2.12,
    "Use get_number_of_weights_key() instead."
  );
  return get_number_of_weights_key();
}

IntKey Weight::get_number_of_weights_key() {
  static IntKey k("nweights");
  return k;
}

FloatKey Weight::get_weight_key(int j) {
  static FloatKeys kk;
  if (kk.empty()) {
    for (int i = 0; i < IMPISD_MAX_WEIGHTS; ++i) {
      std::stringstream out;
      out << i;
      kk.push_back(FloatKey("weight" + out.str()));
    }
  }
  return kk[j];
}

FloatKeys Weight::get_weight_keys() const {
  FloatKeys fks;
  for (unsigned int i = 0; i < get_number_of_weights(); ++i)
    fks.push_back(get_weight_key(i));
  return fks;
}

ObjectKey Weight::get_constraint_key() {
  static ObjectKey k("weight_const");
  return k;
}

Float Weight::get_weight(int i) const {
  IMP_USAGE_CHECK(i < get_number_of_weights(), "Out of range");
  return get_particle()->get_value(get_weight_key(i));
}

algebra::VectorKD Weight::get_weights() const {
  Int nweights = get_number_of_weights();
  algebra::VectorKD w = algebra::get_zero_vector_kd(nweights);
  for (int i = 0; i < nweights; ++i)
    w[i] = get_particle()->get_value(get_weight_key(i));
  return w;
}

void Weight::set_weight_lazy(int i, Float wi) {
  IMP_USAGE_CHECK(static_cast<int>(i) < get_number_of_weights(),
                  "Out of range");
  get_particle()->set_value(get_weight_key(i), wi);
}

void Weight::set_weights_lazy(const algebra::VectorKD& w) {
  Int nweights = w.get_dimension();
  IMP_USAGE_CHECK(static_cast<int>(nweights) == get_number_of_weights(),
                  "Out of range");
  for (unsigned int i = 0; i < nweights; ++i)
    get_particle()->set_value(get_weight_key(i), w[i]);
}

void Weight::set_weights(const algebra::VectorKD& w) {
  Int nweights = w.get_dimension();
  IMP_USAGE_CHECK(static_cast<int>(nweights) == get_number_of_weights(),
                  "Out of range");

  bool project = false;
  Float wsum = 0.0;
  for (unsigned int i = 0; i < nweights; ++i) {
    if (w[i] < 0) {
      project = true;
      break;
    }
    wsum += w[i];
    if (wsum > 1) {
      project = true;
      break;
    }
  }

  if (!project) {
    if (std::abs(wsum - 1.0) < std::numeric_limits<double>::epsilon()) {
      for (unsigned int i = 0; i < nweights; ++i)
        get_particle()->set_value(get_weight_key(i), w[i]);
      return;
    } else if (wsum == 0.0) {
      Float wi = 1.0 / static_cast<Float>(nweights);
      for (unsigned int i = 0; i < nweights; ++i)
        get_particle()->set_value(get_weight_key(i), wi);
      return;
    }
  }

  // Weights are not on the simplex.
  // Perform O(n log(n)) Euclidean projection from arxiv:1309.1541.
  IMP_LOG_VERBOSE("Weight particle " << get_particle()->get_name()
                                     << " has weights " << w
                                     << " with l1 norm " << get_l1_norm(w)
                                     << " and will be projected");

  Floats u(nweights);
  std::copy(w.begin(), w.end(), u.begin());
  std::sort(u.begin(), u.end(), std::greater<double>());
  
  Floats u_cumsum(nweights);
  Float usum = 0.0;
  for (unsigned int i = 0; i < nweights; ++i) {
    usum += u[i];
    u_cumsum[i] = usum;
  }
  int rho = 1;
  while (rho < nweights) {
    if (u[rho] + (1 - u_cumsum[rho]) / (rho + 1) < 0)
      break;
    rho += 1;
  }
  Float lam = (1 - u_cumsum[rho - 1]) / rho;

  for (unsigned int i = 0; i < nweights; ++i) {
    Float wi = w[i] + lam;
    get_particle()->set_value(get_weight_key(i), wi > 0 ? wi : 0.0);
  }
}

bool Weight::get_weights_are_optimized() const {
  for (unsigned int i = 0; i < get_number_of_weights(); ++i){
    if (!get_particle()->get_is_optimized(get_weight_key(i)))
      return false;
  }
  return true;
}

void Weight::set_weights_are_optimized(bool tf) {
  for (unsigned int i = 0; i < get_number_of_weights(); ++i)
    get_particle()->set_is_optimized(get_weight_key(i), tf);
}

Float Weight::get_weight_derivative(int i) const {
  int nweights = get_number_of_weights();
  IMP_USAGE_CHECK(i < nweights, "Out of bounds.");
  return get_particle()->get_derivative(get_weight_key(i));
}

algebra::VectorKD Weight::get_weights_derivatives() const {
  int nweights = get_number_of_weights();
  algebra::VectorKD dw = algebra::get_zero_vector_kd(nweights);
  for (int i = 0; i < nweights; ++i)
    dw[i] = get_particle()->get_derivative(get_weight_key(i));
  return dw;
}

void Weight::add_to_weight_derivative(int i, Float dwi,
                                      const DerivativeAccumulator &da) {
  int nweights = get_number_of_weights();
  IMP_USAGE_CHECK(i < nweights, "Out of bounds.");
  get_particle()->add_to_derivative(get_weight_key(i), dwi, da);
}

void Weight::add_to_weights_derivatives(const algebra::VectorKD& dw,
                                        const DerivativeAccumulator &da) {
  int nweights = dw.get_dimension();
  IMP_USAGE_CHECK(static_cast<int>(nweights) == get_number_of_weights(),
                  "Out of range");
  for (unsigned int i = 0; i < nweights; ++i)
    get_particle()->add_to_derivative(get_weight_key(i), dw[i], da);
}

void Weight::add_weight() {
  IMPISD_DEPRECATED_METHOD_DEF(
    2.12,
    "Set up the Weight with a fixed number of weights instead."
  );
  Int nweights = get_number_of_weights() + 1;
  IMP_USAGE_CHECK(nweights <= IMPISD_MAX_WEIGHTS, "Out of range");
  get_particle()->set_value(get_number_of_weights_key(), nweights);
  Float w = 1.0 / static_cast<Float>(nweights);
  for (int i = 0; i < nweights; ++i)
    get_particle()->set_value(get_weight_key(i), w);
}

Int Weight::get_number_of_states() const {
  IMPISD_DEPRECATED_METHOD_DEF(
    2.12,
    "Use get_number_of_weights() instead."
  );
  return get_number_of_weights();
}

Int Weight::get_number_of_weights() const {
  return get_particle()->get_value(get_number_of_weights_key());
}

void Weight::show(std::ostream &out) const {
  out << "Weight: " << get_weights();
}



void WeightSimplexConstraint::do_update_attributes() {
  Weight w(get_model(), pi_);
  w.set_weights(w.get_weights());
}

void WeightSimplexConstraint::do_update_derivatives(
  DerivativeAccumulator *da) {}

ModelObjectsTemp WeightSimplexConstraint::do_get_inputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

ModelObjectsTemp WeightSimplexConstraint::do_get_outputs() const {
  return ModelObjectsTemp(1, get_model()->get_particle(pi_));
}

IMPISD_END_NAMESPACE
