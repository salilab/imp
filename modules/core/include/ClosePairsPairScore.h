/**
 *  \file IMP/core/ClosePairsPairScore.h
 *  \brief Apply a PairScore to close pairs.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H
#define IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include "XYZR.h"
#include "RigidClosePairsFinder.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>
#include <IMP/internal/container_helpers.h>

IMPCORE_BEGIN_NAMESPACE
//! Apply a score to a fixed number of close pairs from the two sets.
/** Apply the score to the k closest pairs (sphere distance). The
    envisioned use case is that the two particles each represent a protein
    and the refiners return the geometry for that protein.

    Rigid bodies are special cased for efficiency.
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT KClosePairsPairScore : public PairScore {
  IMP::PointerMember<Refiner> r_;
  IMP::PointerMember<PairScore> f_;
  int k_;
  mutable double last_distance_;
  IMP::PointerMember<RigidClosePairsFinder> cpf_;
  ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexPair &pp) const;

 public:
  //! only score the k closest pairs.
  KClosePairsPairScore(PairScore *f, Refiner *r, int k = 1);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp)
      const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }

  Restraints create_current_decomposition(
      Model *m, const ParticleIndexPair &vt) const;

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual double evaluate_if_good_index(Model *m,
                                        const ParticleIndexPair &vt,
                                        DerivativeAccumulator *da,
                                        double max) const IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(KClosePairsPairScore);
  IMP_OBJECT_METHODS(KClosePairsPairScore);
};

//! Apply the score to all pairs whose spheres are within a distance threshold.
/** Rigid bodies are special cased for efficiency.

    \see ClosePairsScoreState
 */
class IMPCOREEXPORT ClosePairsPairScore : public PairScore {
  IMP::PointerMember<Refiner> r_;
  IMP::PointerMember<PairScore> f_;
  Float th_;
  IMP::PointerMember<RigidClosePairsFinder> cpf_;
  ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexPair &pp) const;

 public:
  //! Constructor.
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
      \param[in] max_distance Only score pairs which are close than
      the max_distance
   */
  ClosePairsPairScore(PairScore *f, Refiner *r, Float max_distance);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp)
      const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }
  Restraints create_current_decomposition(
      Model *m, const ParticleIndexPair &vt) const;

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual double evaluate_if_good_index(Model *m,
                                        const ParticleIndexPair &vt,
                                        DerivativeAccumulator *da,
                                        double max) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_PAIR_SCORE_METHODS(KClosePairsPairScore);
  IMP_OBJECT_METHODS(ClosePairsPairScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H */
