/**
 *  \file IMP/core/GridClosePairsFinder.h
 *  \brief Use a hierarchy of grids to find close pairs.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_GRID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_GRID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include <IMP/base/object_macros.h>
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/**
   \see CloserPairsScoreState
 */
class IMPCOREEXPORT GridClosePairsFinder : public ClosePairsFinder {
 public:
  GridClosePairsFinder();
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

  IMP_OBJECT_METHODS(GridClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GRID_CLOSE_PAIRS_FINDER_H */
