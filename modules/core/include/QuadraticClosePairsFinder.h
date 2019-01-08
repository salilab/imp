/**
 *  \file IMP/core/QuadraticClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H
#define IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/** \see ClosePairsScoreState
 */
class IMPCOREEXPORT QuadraticClosePairsFinder : public ClosePairsFinder {
 public:
  QuadraticClosePairsFinder();
  //! returns true if a and b are close, and pass any filter added
  //! to this
  bool get_are_close_and_filtered(Particle *a, Particle *b) const {
    return get_are_close_and_filtered
      (a->get_model(),
       a->get_index(),
       b->get_index());
  }
  //! returns true if a and b of m are close, and pass any filter added
  //! to this
  bool get_are_close_and_filtered(Model *m, ParticleIndex a,
                     ParticleIndex b) const;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pc) const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pca,
      const ParticleIndexes &pcb) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(QuadraticClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H */
