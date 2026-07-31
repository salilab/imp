/**
 *  \file IMP/atom/LennardJonesTypedPairScore.h
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H
#define IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/generic.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include <IMP/Pointer.h>
#include <IMP/atom/LennardJonesType.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

//! Lennard-Jones score between a pair of particles.
/** The two particles in the pair must be LennardJonesTyped particles.
    The form of the potential is \f[
       -\epsilon \left[ w_{rep} \left(\frac{r_{min}}{r}\right)^{12}
                       - 2 w_{att} \left(\frac{r_{min}}{r}\right)^{6}\right]
    \f] where \f$\epsilon\f$ is the depth of the well between the
    two particles, \f$r_{min}\f$ the distance corresponding to the minimum
    score, \f$r\f$ the inter-particle distance, and \f$w_{rep}\f$ and
    \f$w_{att}\f$ the weights on the repulsive and attractive parts of the
    potential respectively; both weights are 1.0 by default.

    The well depth is the geometric mean of the individual particles' well
    depths (as extracted by LennardJonesType::get_well_depth) and the minimum
    distance the sum of the particles' Lennard-Jones radii (as extracted
    by LennardJonesType::get_radius; note that this is not necessarily the
    same as the core::XYZR radius).
 */
template <class SmoothingFuncT>
class LennardJonesTypedPairScore final : public PairScore {
  IMP::PointerMember<SmoothingFuncT> smoothing_function_;
  double repulsive_weight_, attractive_weight_;
  IMP::PointerMember<internal::LennardJonesParameters> params_;

  double evaluate_index_fast(Model *m,
                             const ParticleIndexPair &p,
                             DerivativeAccumulator *da,
                             const int *type_array) const;

public:
  LennardJonesTypedPairScore(SmoothingFuncT *f)
      : smoothing_function_(f),
        repulsive_weight_(1.0),
        attractive_weight_(1.0) {
    params_ = internal::get_lj_params();
  }

  void set_repulsive_weight(double repulsive_weight) {
    repulsive_weight_ = repulsive_weight;
  }

  double get_repulsive_weight() const { return repulsive_weight_; }

  void set_attractive_weight(double attractive_weight) {
    attractive_weight_ = attractive_weight;
  }

  double get_attractive_weight() const { return attractive_weight_; }

  //! Return the precalculated repulsive factors for each pair of LJ types
  /** This will become invalid if new types are added or well depths
      or radii are changed. */
  Floats get_repulsive_type_factors() const { return params_->aij_; }

  //! Return the precalculated attractive factors for each pair of LJ types
  /** This will become invalid if new types are added or well depths
      or radii are changed. */
  Floats get_attractive_type_factors() const { return params_->bij_; }

  SmoothingFuncT *get_smoothing_function() const {
    return smoothing_function_;
  }

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual bool check_indexes(Model *m,
                             const ParticleIndexes &pis) const override;
  IMP_PAIR_SCORE_METHODS_UNCHECKED(LennardJonesTypedPairScore,
      const int *type_array = LennardJonesTyped::get_type_array(m),
      evaluate_index_fast(m, p[i], da, type_array) );
  IMP_OBJECT_METHODS(LennardJonesTypedPairScore);
  ;
};

template <class SmoothingFuncT>
Float LennardJonesTypedPairScore<SmoothingFuncT>::evaluate_index(
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

template <class SmoothingFuncT>
bool LennardJonesTypedPairScore<SmoothingFuncT>::check_indexes(
      Model *m, const ParticleIndexes &pis) const {
  int total = 0;
  // Every particle should be a LennardJonesTyped particle
  for (ParticleIndex pi: pis) {
    LennardJonesTyped lj0(m, pi);
    // make sure we actually use the object so the compiler doesn't optimize
    // it out
    total += lj0.get_index();
  }
  return true;
}

template <class SmoothingFuncT>
Float LennardJonesTypedPairScore<SmoothingFuncT>::evaluate_index_fast(
      Model *m, const ParticleIndexPair &p, DerivativeAccumulator *da,
      const int *type_array) const {
  core::XYZ lj0(m, std::get<0>(p));
  core::XYZ lj1(m, std::get<1>(p));
  int type0 = type_array[lj0.get_particle_index().get_index()];
  int type1 = type_array[lj1.get_particle_index().get_index()];

  algebra::Vector3D delta = lj0.get_coordinates() - lj1.get_coordinates();
  double distsqr = delta.get_squared_magnitude();
  double dist = std::sqrt(distsqr);
  double dist6 = distsqr * distsqr * distsqr;
  double dist12 = dist6 * dist6;

  int index = params_->get_parameter_index(type0, type1);
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

template <class SmoothingFuncT>
ModelObjectsTemp LennardJonesTypedPairScore<SmoothingFuncT>::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H */
