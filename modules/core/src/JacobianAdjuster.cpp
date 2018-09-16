/**
 *  \file JacobianAdjuster.cpp
 *  \brief Classes for dealing with transformations of parameters.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/JacobianAdjuster.h>
#include <IMP/object_cast.h>

IMPCORE_BEGIN_NAMESPACE

JacobianAdjuster::JacobianAdjuster(Model* m, const std::string name) 
  : ModelObject(m, name), temp_(1) {}

void JacobianAdjuster::set_jacobian(FloatKey k, ParticleIndex pi,
                                    const UnivariateJacobian& j) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    IMP_USAGE_CHECK(get_model()->get_has_attribute(k, pi),
                    "Particle does not have Float attribute.");
  }

  FloatIndex fi(pi, k);
  UnivariateJacobianMap::iterator lb = uni_map_.lower_bound(fi);
  if(lb != uni_map_.end() && !(uni_map_.key_comp()(fi, lb->first))) {
    lb->second = j;
  } else {
    uni_map_.insert(lb, UnivariateJacobianMap::value_type(fi, j));
    set_has_dependencies(false);
    create_score_state();
  }
}

void JacobianAdjuster::set_jacobian(
    FloatKeys ks, ParticleIndex pi, const MultivariateJacobian& j) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    int N = ks.size();
    IMP_USAGE_CHECK(
        N > 0,
        "Multivariate transformed parameter must at least one key.");
    IMP_USAGE_CHECK(
      j.get_jacobian().size() == N && j.get_gradient_adjustment().size() == N,
      "Jacobian and gradient adjustment dimensions must equal number of keys.");
    IMP_FOREACH(FloatKey k, ks) {
      IMP_USAGE_CHECK(get_model()->get_has_attribute(k, pi),
                      "Particle does not have Float attribute.");
    }
  }

  FloatsIndex fi(pi, ks);
  MultivariateJacobianMap::iterator lb = multi_map_.lower_bound(fi);
  if(lb != multi_map_.end() && !(multi_map_.key_comp()(fi, lb->first))) {
    lb->second = j;
  } else {
    multi_map_.insert(lb, MultivariateJacobianMap::value_type(fi, j));
    set_has_dependencies(false);
    create_score_state();
  }
}

const UnivariateJacobian& JacobianAdjuster::get_jacobian(FloatKey k, ParticleIndex pi) const {
  FloatIndex fi(pi, k);
  UnivariateJacobianMap::const_iterator it;
  it = uni_map_.find(fi);
  IMP_USAGE_CHECK(it != uni_map_.end(), "Jacobian has not been added.");
  return it->second;
}

UnivariateJacobian& JacobianAdjuster::access_jacobian(FloatKey k, ParticleIndex pi) {
  FloatIndex fi(pi, k);
  UnivariateJacobianMap::iterator it;
  it = uni_map_.find(fi);
  IMP_USAGE_CHECK(it != uni_map_.end(), "Jacobian has not been added.");
  return it->second;
}

const MultivariateJacobian& JacobianAdjuster::get_jacobian(FloatKeys ks, ParticleIndex pi) const {
  FloatsIndex fi(pi, ks);
  MultivariateJacobianMap::const_iterator it;
  it = multi_map_.find(fi);
  IMP_USAGE_CHECK(it != multi_map_.end(), "Jacobian has not been added.");
  return it->second;
}

MultivariateJacobian& JacobianAdjuster::access_jacobian(FloatKeys ks, ParticleIndex pi) {
  FloatsIndex fi(pi, ks);
  MultivariateJacobianMap::iterator it;
  it = multi_map_.find(fi);
  IMP_USAGE_CHECK(it != multi_map_.end(), "Jacobian has not been added.");
  return it->second;
}

void JacobianAdjuster::set_temperature(double temperature) {
  IMP_INTERNAL_CHECK(temperature > 0, "Temperature must be positive.");
  temp_ = temperature;
}

double JacobianAdjuster::get_temperature() const { return temp_; }

double JacobianAdjuster::get_score_adjustment() const {
  double adj = 0;
  Model *m = get_model();

  FloatIndex fi;
  IMP_FOREACH(UP up, uni_map_) {
    fi = up.first;
    if (m->get_is_optimized(fi.get_key(), fi.get_particle())) {
      adj += up.second.get_score_adjustment();
    }
  }

  FloatsIndex fsi;
  IMP_FOREACH(MP mp, multi_map_) {
    fsi = mp.first;
    if (m->get_is_optimized(fsi.get_keys()[0], fsi.get_particle())) {
      adj += mp.second.get_score_adjustment();
    }
  }

  return adj * temp_;
}

void JacobianAdjuster::apply_gradient_adjustment() {
  Model *m = get_model();
  DerivativeAccumulator da = DerivativeAccumulator(temp_);

  FloatIndex fi;
  IMP_FOREACH(UP up, uni_map_) {
    fi = up.first;
    if (m->get_is_optimized(fi.get_key(), fi.get_particle())) {
        m->add_to_derivative(fi.get_key(), fi.get_particle(),
                             up.second.get_gradient_adjustment(), da);
    }
  }

  FloatsIndex fsi;
  ParticleIndex pi;
  Floats grad_adj;
  FloatKeys ks;
  unsigned int n, N;
  IMP_FOREACH(MP mp, multi_map_) {
    fsi = mp.first;
    FloatKeys ks = fsi.get_keys();
    pi = fsi.get_particle();
    if (m->get_is_optimized(ks[0], pi)) {
      grad_adj = mp.second.get_gradient_adjustment();
      N = ks.size();
      IMP_INTERNAL_CHECK(
        grad_adj.size() == N,
        "Size of gradient doesn't match number of attributes.");
      for (n = 0; n < N; ++n) {
        m->add_to_derivative(ks[n], pi, grad_adj[n], da);
      }
    }
  }
}

ModelKey JacobianAdjuster::get_score_state_key() {
  static ModelKey k("jacobian adjust gradient score state");
  return k;
}

void JacobianAdjuster::create_score_state() {
  ModelKey k = get_score_state_key();
  if (!get_model()->get_has_data(k)) {
    IMP_NEW(JacobianAdjustGradient, ss, (get_model()));
    get_model()->add_data(k, ss);
    get_model()->add_score_state(ss.release());
  } else {
    IMP::Pointer<JacobianAdjustGradient> ss =
        IMP::object_cast<JacobianAdjustGradient>(get_model()->get_data(k));
    ss->set_has_dependencies(false);
  }
}

ModelObjectsTemp JacobianAdjuster::do_get_inputs() const {
  ModelObjectsTemp ret;
  Model *m = get_model();

  ret.reserve(uni_map_.size() + multi_map_.size());

  IMP_FOREACH(UP up, uni_map_) {
    ret.push_back(m->get_particle(up.first.get_particle()));
  }

  IMP_FOREACH(MP mp, multi_map_) {
    ret.push_back(m->get_particle(mp.first.get_particle()));
  }

  return ret;
}

ModelObjectsTemp JacobianAdjuster::do_get_outputs() const {
  return get_inputs();
}

JacobianAdjuster *get_jacobian_adjuster(Model *m) {
  static ModelKey k("jacobian adjuster");
  if (!m->get_has_data(k)) {
    IMP_NEW(JacobianAdjuster, ja, (m));
    m->add_data(k, ja);
    return ja.release();
  } else {
    IMP::Pointer<JacobianAdjuster> ja =
        IMP::object_cast<JacobianAdjuster>(m->get_data(k));
    return ja.release();
  }
}

void JacobianAdjustGradient::do_before_evaluate() {}

void JacobianAdjustGradient::do_after_evaluate(DerivativeAccumulator *da) {
  get_jacobian_adjuster(get_model())->apply_gradient_adjustment();
}

ModelObjectsTemp JacobianAdjustGradient::do_get_inputs() const {
  return get_jacobian_adjuster(get_model())->get_inputs();
}

ModelObjectsTemp JacobianAdjustGradient::do_get_outputs() const {
  return get_jacobian_adjuster(get_model())->get_outputs();
}

IMPCORE_END_NAMESPACE
