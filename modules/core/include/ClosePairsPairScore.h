/**
 *  \file IMP/core/ClosePairsPairScore.h
 *  \brief Apply a PairScore to close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H
#define IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include "XYZR.h"
#include "RigidClosePairsFinder.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base/Pointer.h>
#include <IMP/Refiner.h>
#include <IMP/kernel/internal/container_helpers.h>

IMPCORE_BEGIN_NAMESPACE
/** Apply a score to a fixed number of close pairs from the two sets.

    Apply the score to either the k closest pairs (sphere distance). The
    envisioned use case is that the two particles each represent a protein
    and the refiners return the geometry for that protein.
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT KClosePairsPairScore : public PairScore {
  IMP::base::PointerMember<Refiner> r_;
  IMP::base::PointerMember<PairScore> f_;
  int k_;
  mutable double last_distance_;
  IMP::base::PointerMember<RigidClosePairsFinder> cpf_;
  kernel::ParticleIndexPairs get_close_pairs(kernel::Model *m,
                                     const kernel::ParticleIndexPair &pp) const;

 public:
  /** only score the k closest pairs.
   */
  KClosePairsPairScore(PairScore *f, Refiner *r, int k = 1);

  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlePair &pp) const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }

  kernel::Restraints create_current_decomposition(kernel::Model *m,
                                          const kernel::ParticleIndexPair &vt) const;

  virtual double evaluate_index(kernel::Model *m, const kernel::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;
  virtual double evaluate_if_good_index(kernel::Model *m, const kernel::ParticleIndexPair &vt,
                                        DerivativeAccumulator *da,
                                        double max) const IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(KClosePairsPairScore);
  IMP_OBJECT_METHODS(KClosePairsPairScore);
};

/** Apply the score to all pairs whose
    spheres are within a certain distance threshold.

    \see ClosePairsScoreState
 */
class IMPCOREEXPORT ClosePairsPairScore : public PairScore {
  IMP::base::PointerMember<Refiner> r_;
  IMP::base::PointerMember<PairScore> f_;
  Float th_;
  IMP::base::PointerMember<RigidClosePairsFinder> cpf_;
  kernel::ParticleIndexPairs get_close_pairs(kernel::Model *m,
                                     const kernel::ParticleIndexPair &pp) const;

 public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
      \param[in] max_distance Only score pairs which are close than
      the max_distance
   */
  ClosePairsPairScore(PairScore *f, Refiner *r, Float max_distance);

  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlePair &pp) const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }
  kernel::Restraints create_current_decomposition(kernel::Model *m,
                                          const kernel::ParticleIndexPair &vt) const;

  virtual double evaluate_index(kernel::Model *m, const kernel::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual double evaluate_if_good_index(kernel::Model *m, const kernel::ParticleIndexPair &vt,
                                        DerivativeAccumulator *da,
                                        double max) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(kernel::Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(KClosePairsPairScore);
  IMP_OBJECT_METHODS(ClosePairsPairScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H */
