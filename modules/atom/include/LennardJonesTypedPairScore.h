/**
 *  \file IMP/atom/LennardJonesTypedPairScore.h
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2025 IMP Inventors. All rights reserved.
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
class IMPATOMEXPORT LennardJonesTypedPairScore : public PairScore {
  IMP::PointerMember<atom::SmoothingFunction> smoothing_function_;
  double repulsive_weight_, attractive_weight_;
  IMP::PointerMember<internal::LennardJonesParameters> params_;

public:
  LennardJonesTypedPairScore(atom::SmoothingFunction *f)
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

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_PAIR_SCORE_METHODS(LennardJonesTypedPairScore);
  IMP_OBJECT_METHODS(LennardJonesTypedPairScore);
  ;
};

IMP_OBJECTS(LennardJonesTypedPairScore, LennardJonesTypedPairScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_TYPED_PAIR_SCORE_H */
