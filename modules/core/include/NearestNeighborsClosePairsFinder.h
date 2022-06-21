/**
 *  \file IMP/core/NearestNeighborsClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H
#define IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs using the algebra::NearestNeighbor code
/** \see IMP::container::ClosePairsScoreState
    \ingroup cgal
*/
class IMPCOREEXPORT NearestNeighborsClosePairsFinder : public ClosePairsFinder {
 public:
  NearestNeighborsClosePairsFinder();

  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      override;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      override;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pc) const override;
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pca,
      const ParticleIndexes &pcb) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;

  IMP_OBJECT_METHODS(NearestNeighborsClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H */
