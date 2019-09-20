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
  m->add_attribute(get_number_of_weights_key(), pi, 0);
  add_constraint(m, pi);
}

void Weight::do_setup_particle(Model *m, ParticleIndex pi, Int nweights) {
  Weight pw = setup_particle(m, pi);
  pw.set_number_of_weights_lazy(nweights);
  pw.set_weights_lazy(pw.get_unit_simplex().get_barycenter());
}

void Weight::do_setup_particle(Model *m, ParticleIndex pi,
                               const algebra::VectorKD &w) {
  Weight pw = setup_particle(m, pi);
  pw.set_number_of_weights_lazy(w.get_dimension());
  pw.set_weights(w);
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
  set_weights_lazy(algebra::get_projected(get_unit_simplex(), w));
}

bool Weight::get_weights_are_optimized() const {
  Int nweights = get_number_of_weights();
  if (nweights == 0) return false;
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

void Weight::set_number_of_weights_lazy(Int nweights) {
  IMP_USAGE_CHECK(nweights > 0, "Number of weights must be greater than zero.");
  IMP_USAGE_CHECK(nweights <= IMPISD_MAX_WEIGHTS,
                  "Number of weights exceeds the maximum allowed number of "
                  << IMPISD_MAX_WEIGHTS << ".");

  Int old_nweights = get_number_of_weights();
  if (nweights > old_nweights) {
    IMP_USAGE_CHECK(!get_weights_are_optimized(),
                    "Number of weights cannot be changed for optimized Weight");
    for (int i = old_nweights; i < nweights; ++i) {
      if (!get_model()->get_has_attribute(get_weight_key(i),
                                          get_particle_index())) {
        get_model()->add_attribute(get_weight_key(i),
                                   get_particle_index(), 0);
      } else {
        get_particle()->set_value(get_weight_key(i), 0);
      }
    }
    get_particle()->set_value(get_number_of_weights_key(), nweights);
  } else if (nweights < old_nweights) {
    IMP_USAGE_CHECK(!get_weights_are_optimized(),
                    "Number of weights cannot be changed for optimized Weight");
    get_particle()->set_value(get_number_of_weights_key(), nweights);
  }
}

void Weight::set_number_of_weights(Int nweights) {
  set_number_of_weights_lazy(nweights);
  set_weights(get_weights());
}

void Weight::add_weight_lazy(Float wi) {
  Int old_nweights = get_number_of_weights();
  set_number_of_weights_lazy(old_nweights + 1);
  get_particle()->set_value(get_weight_key(old_nweights), wi);
}

void Weight::add_weight(Float wi) {
  add_weight_lazy(wi);
  set_weights(get_weights());
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

algebra::UnitSimplexKD Weight::get_unit_simplex() const {
  return algebra::UnitSimplexKD(get_number_of_weights());
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
