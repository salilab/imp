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
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>
#include <IMP/internal/container_helpers.h>

IMPCORE_BEGIN_NAMESPACE
/** Apply a score to a fixed number of close pairs from the two sets.

    Apply the score to either the k closest pairs (sphere distance). The
    envisioned use case is that the two particles each represent a protein
    and the refiners return the geometry for that protein.
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT KClosePairsPairScore : public PairScore {
  IMP::OwnerPointer<Refiner> r_;
  IMP::OwnerPointer<PairScore> f_;
  int k_;
  mutable double last_distance_;
  IMP::OwnerPointer<RigidClosePairsFinder> cpf_;
  ParticleIndexPairs get_close_pairs(Model *m,
                                     const ParticleIndexPair &pp) const;

 public:
  /** only score the k closest pairs.
   */
  KClosePairsPairScore(PairScore *f, Refiner *r, int k = 1);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp) const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }

  Restraints create_current_decomposition(Model *m,
                                          const ParticleIndexPair &vt) const;

  IMP_COMPOSITE_PAIR_SCORE(KClosePairsPairScore);
};

/** Apply the score to all pairs whose
    spheres are within a certain distance threshold.

    \see ClosePairsScoreState
 */
class IMPCOREEXPORT ClosePairsPairScore : public PairScore {
  IMP::OwnerPointer<Refiner> r_;
  IMP::OwnerPointer<PairScore> f_;
  Float th_;
  IMP::OwnerPointer<RigidClosePairsFinder> cpf_;
  ParticleIndexPairs get_close_pairs(Model *m,
                                     const ParticleIndexPair &pp) const;

 public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
      \param[in] max_distance Only score pairs which are close than
      the max_distance
   */
  ClosePairsPairScore(PairScore *f, Refiner *r, Float max_distance);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp) const {
    return IMP::internal::get_particle(
        pp[0]->get_model(),
        get_close_pairs(pp[0]->get_model(), IMP::internal::get_index(pp)));
  }
  Restraints create_current_decomposition(Model *m,
                                          const ParticleIndexPair &vt) const;

  IMP_COMPOSITE_PAIR_SCORE(ClosePairsPairScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H */
