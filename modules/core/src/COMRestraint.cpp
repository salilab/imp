/**
 *  \file COMRestraint.cpp
 *  \brief Restrain weighted centroids of particle groups.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/COMRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <boost/unordered_set.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

COMRestraint::COMRestraint(Model *m, std::string name)
    : Restraint(m, name), cache_valid_(false) {}

void COMRestraint::check_group(unsigned group) const {
  IMP_USAGE_CHECK(group < groups_.size(),
                  "No group " << group << "; there are " << groups_.size());
}

unsigned COMRestraint::add_group(const ParticleIndexes &pis,
                                 const Floats &weights) {
  IMP_USAGE_CHECK(!pis.empty(), "A group needs at least one particle");
  IMP_USAGE_CHECK(weights.size() == pis.size(),
                  "Got " << pis.size() << " particles but " << weights.size()
                         << " weights");
  Model *m = get_model();
  double total = 0.;
  for (unsigned i = 0; i < pis.size(); ++i) {
    IMP_USAGE_CHECK(core::XYZ::get_is_setup(m, pis[i]),
                    "Particle " << m->get_particle_name(pis[i])
                                << " has no coordinates");
    IMP_USAGE_CHECK(weights[i] >= 0.,
                    "Weight " << weights[i] << " of particle "
                              << m->get_particle_name(pis[i])
                              << " is negative");
    total += weights[i];
  }
  IMP_USAGE_CHECK(total > 0., "The weights of a group are all zero");
  Floats normalized(weights);
  for (double &w : normalized) w /= total;
  groups_.push_back(pis);
  weights_.push_back(normalized);
  return groups_.size() - 1;
}

unsigned COMRestraint::add_group(const ParticleIndexes &pis) {
  return add_group(pis, Floats(pis.size(), 1.));
}

unsigned COMRestraint::add_term(Term t) {
  terms_.push_back(t);
  unsigned index = terms_.size() - 1;
  // map every particle of the groups, and the rigid body of a rigid
  // member, to this term: a mover reports the rigid body, not its members
  Model *m = get_model();
  boost::unordered_set<ParticleIndex> keys;
  std::vector<unsigned> groups(1, t.group1);
  if (t.type == PAIR) groups.push_back(t.group2);
  for (unsigned g : groups) {
    for (ParticleIndex pi : groups_[g]) {
      keys.insert(pi);
      if (core::RigidMember::get_is_setup(m, pi)) {
        keys.insert(core::RigidMember(m, pi).get_rigid_body()
                        .get_particle_index());
      } else if (core::NonRigidMember::get_is_setup(m, pi)) {
        keys.insert(core::NonRigidMember(m, pi).get_rigid_body()
                        .get_particle_index());
      }
    }
  }
  for (ParticleIndex pi : keys) affected_terms_[pi].push_back(index);
  cache_valid_ = false;
  return index;
}

unsigned COMRestraint::add_point_term(unsigned group,
                                      const algebra::Vector3D &point,
                                      UnaryFunction *f) {
  check_group(group);
  Term t;
  t.type = POINT;
  t.group1 = t.group2 = group;
  t.point = point;
  t.f = f;
  return add_term(t);
}

unsigned COMRestraint::add_plane_term(unsigned group,
                                      const algebra::Vector3D &point,
                                      const algebra::Vector3D &normal,
                                      UnaryFunction *f) {
  check_group(group);
  IMP_USAGE_CHECK(normal.get_magnitude() > 0., "The normal must not be zero");
  Term t;
  t.type = PLANE;
  t.group1 = t.group2 = group;
  t.point = point;
  t.direction = normal.get_unit_vector();
  t.f = f;
  return add_term(t);
}

unsigned COMRestraint::add_axis_term(unsigned group,
                                     const algebra::Vector3D &point,
                                     const algebra::Vector3D &direction,
                                     UnaryFunction *f) {
  check_group(group);
  IMP_USAGE_CHECK(direction.get_magnitude() > 0.,
                  "The direction must not be zero");
  Term t;
  t.type = AXIS;
  t.group1 = t.group2 = group;
  t.point = point;
  t.direction = direction.get_unit_vector();
  t.f = f;
  return add_term(t);
}

unsigned COMRestraint::add_pair_term(unsigned group1, unsigned group2,
                                     UnaryFunction *f) {
  check_group(group1);
  check_group(group2);
  Term t;
  t.type = PAIR;
  t.group1 = group1;
  t.group2 = group2;
  t.f = f;
  return add_term(t);
}

algebra::Vector3D COMRestraint::get_centroid_of(unsigned group) const {
  Model *m = get_model();
  algebra::Vector3D c(0., 0., 0.);
  const ParticleIndexes &pis = groups_[group];
  const Floats &w = weights_[group];
  for (unsigned i = 0; i < pis.size(); ++i) {
    c += w[i] * core::XYZ(m, pis[i]).get_coordinates();
  }
  return c;
}

algebra::Vector3D COMRestraint::get_center_of_mass(unsigned group) const {
  check_group(group);
  return get_centroid_of(group);
}

// The quantity a term scores, and its gradient with respect to the COM of
// group1 (grad1) and, for a pair term, group2 (grad2).
double COMRestraint::get_feature(const Term &t, algebra::Vector3D &grad1,
                                 algebra::Vector3D &grad2) const {
  const double eps = 1e-9;
  algebra::Vector3D c = get_centroid_of(t.group1);
  grad2 = algebra::Vector3D(0., 0., 0.);
  switch (t.type) {
    case POINT: {
      algebra::Vector3D d = c - t.point;
      double r = d.get_magnitude();
      grad1 = r > eps ? d / r : algebra::Vector3D(0., 0., 0.);
      return r;
    }
    case PLANE: {
      grad1 = t.direction;
      return (c - t.point) * t.direction;
    }
    case AXIS: {
      algebra::Vector3D d = c - t.point;
      algebra::Vector3D radial = d - (d * t.direction) * t.direction;
      double r = radial.get_magnitude();
      grad1 = r > eps ? radial / r : algebra::Vector3D(0., 0., 0.);
      return r;
    }
    case PAIR:
    default: {
      algebra::Vector3D d = c - get_centroid_of(t.group2);
      double r = d.get_magnitude();
      grad1 = r > eps ? d / r : algebra::Vector3D(0., 0., 0.);
      grad2 = -grad1;
      return r;
    }
  }
}

double COMRestraint::evaluate_term(unsigned i,
                                   DerivativeAccumulator *accum) const {
  const Term &t = terms_[i];
  algebra::Vector3D grad1, grad2;
  double feature = get_feature(t, grad1, grad2);
  if (!accum) return t.f->evaluate(feature);
  DerivativePair sd = t.f->evaluate_with_derivative(feature);
  // d score / d x_i = f'(feature) * d feature / d COM * m_i / M
  Model *m = get_model();
  for (unsigned g = 0; g < (t.type == PAIR ? 2u : 1u); ++g) {
    unsigned group = g == 0 ? t.group1 : t.group2;
    const algebra::Vector3D &grad = g == 0 ? grad1 : grad2;
    const ParticleIndexes &pis = groups_[group];
    const Floats &w = weights_[group];
    for (unsigned k = 0; k < pis.size(); ++k) {
      core::XYZ(m, pis[k]).add_to_derivatives(sd.second * w[k] * grad, *accum);
    }
  }
  return sd.first;
}

double COMRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const {
  term_scores_.resize(terms_.size());
  double score = 0.;
  for (unsigned i = 0; i < terms_.size(); ++i) {
    term_scores_[i] = evaluate_term(i, accum);
    score += term_scores_[i];
  }
  cache_valid_ = true;
  return score;
}

double COMRestraint::unprotected_evaluate_moved(
    DerivativeAccumulator *accum, const ParticleIndexes &moved_pis,
    const ParticleIndexes &reset_pis) const {
  if (accum || !cache_valid_) return unprotected_evaluate(accum);
  // recompute only the terms of groups holding a moved (or reset) particle;
  // a particle this restraint cannot place forces a full evaluation
  boost::unordered_set<unsigned> stale;
  for (const ParticleIndexes *pis : {&moved_pis, &reset_pis}) {
    for (ParticleIndex pi : *pis) {
      auto it = affected_terms_.find(pi);
      if (it == affected_terms_.end()) return unprotected_evaluate(accum);
      stale.insert(it->second.begin(), it->second.end());
    }
  }
  double score = 0.;
  for (unsigned i = 0; i < terms_.size(); ++i) {
    if (stale.find(i) != stale.end()) {
      term_scores_[i] = evaluate_term(i, nullptr);
    }
    score += term_scores_[i];
  }
  return score;
}

ModelObjectsTemp COMRestraint::do_get_inputs() const {
  Model *m = get_model();
  ModelObjectsTemp ret;
  for (const ParticleIndexes &pis : groups_) {
    for (ParticleIndex pi : pis) ret.push_back(m->get_particle(pi));
  }
  return ret;
}

SoftLowerBound::SoftLowerBound(double softness)
    : UnaryFunction("SoftLowerBound%1%"), softness_(softness) {
  IMP_USAGE_CHECK(softness > 0., "softness must be > 0");
}

double SoftLowerBound::evaluate(double feature) const {
  // log(1 + exp(u)), u = -feature / softness, stable for large |u|
  double u = -feature / softness_;
  return std::max(u, 0.) + std::log1p(std::exp(-std::abs(u)));
}

DerivativePair SoftLowerBound::evaluate_with_derivative(double feature) const {
  double u = -feature / softness_;
  double sigmoid = u >= 0 ? 1. / (1. + std::exp(-u))
                          : std::exp(u) / (1. + std::exp(u));
  return DerivativePair(evaluate(feature), -sigmoid / softness_);
}

IMPCORE_END_NAMESPACE
