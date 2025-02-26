/**
 *  \file LennardJonesTypedPairScore.cpp
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/LennardJonesTypedPairScore.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

LennardJonesType::LennardJonesType(double well_depth, double radius,
                                   std::string name)
                : Object(name), well_depth_(well_depth), radius_(radius) {
  index_ = internal::get_lj_params()->add(this);
}

void LennardJonesType::set_well_depth(double d) {
  well_depth_ = d;
  internal::get_lj_params()->precalculate(index_);
}

void LennardJonesType::set_radius(double r) {
  radius_ = r;
  internal::get_lj_params()->precalculate(index_);
}

IntKey LennardJonesTyped::get_type_key() {
  static IntKey k("lennard_jones_type");
  return k;
}

void LennardJonesTyped::show(std::ostream &out) const {
  XYZ::show(out);
  out << " Lennard-Jones type= " << get_index();
}

Float LennardJonesTypedPairScore::evaluate_index(
      Model *m, const ParticleIndexPair &p, DerivativeAccumulator *da) const {
  LennardJonesTyped lj0(m, std::get<0>(p));
  LennardJonesTyped lj1(m, std::get<1>(p));

  algebra::Vector3D delta = lj0.get_coordinates() - lj1.get_coordinates();
  double distsqr = delta.get_squared_magnitude();
  double dist = std::sqrt(distsqr);
  double dist6 = distsqr * distsqr * distsqr;
  double dist12 = dist6 * dist6;

  int index = params_->get_parameter_index(lj0.get_index(), lj1.get_index());
  double A = params_->aij_[index] * repulsive_weight_;
  double B = params_->bij_[index] * attractive_weight_;
  double repulsive = A / dist12;
  double attractive = B / dist6;
  double score = repulsive - attractive;

  if (da) {
    DerivativePair d = (*smoothing_function_)(
        score, (6.0 * attractive - 12.0 * repulsive) / dist, dist);
    algebra::Vector3D deriv = d.second * delta / dist;
    lj0.add_to_derivatives(deriv, *da);
    lj1.add_to_derivatives(-deriv, *da);
    return d.first;
  } else {
    return (*smoothing_function_)(score, dist);
  }
}

ModelObjectsTemp LennardJonesTypedPairScore::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
