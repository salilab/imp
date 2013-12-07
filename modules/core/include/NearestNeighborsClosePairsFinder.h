/**
 *  \file IMP/core/NearestNeighborsClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H
#define IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs using the algebra::NearestNeighbor code
/** \see IMP::container::ClosePairsScoreState
    \ingroup cgal
*/
class IMPCOREEXPORT NearestNeighborsClosePairsFinder : public ClosePairsFinder {
 public:
  NearestNeighborsClosePairsFinder();

#ifndef SWIG
  using ClosePairsFinder::get_close_pairs;
#else
  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlesTemp &pc)
      const;
  kernel::ParticlePairsTemp get_close_pairs(
      const kernel::ParticlesTemp &pca, const kernel::ParticlesTemp &pcb) const;
#endif
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_close_pairs(
      kernel::Model *m, const kernel::ParticleIndexes &pc) const IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_close_pairs(
      kernel::Model *m, const kernel::ParticleIndexes &pca,
      const kernel::ParticleIndexes &pcb) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(NearestNeighborsClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_NEAREST_NEIGHBORS_CLOSE_PAIRS_FINDER_H */
