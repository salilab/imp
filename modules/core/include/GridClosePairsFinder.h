/**
 *  \file IMP/core/GridClosePairsFinder.h
 *  \brief Use a hierarchy of grids to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_GRID_CLOSE_PAIRS_FINDER_H
#define IMPCORE_GRID_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"
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

  virtual ParticlePairsTemp
  get_close_pairs(const ParticlesTemp &pa) const IMP_OVERRIDE;
  virtual ParticlePairsTemp get_close_pairs(
      const ParticlesTemp &pa, const ParticlesTemp &pb) const IMP_OVERRIDE;
  virtual IntPairs
  get_close_pairs(const algebra::BoundingBox3Ds &bbs) const IMP_OVERRIDE;
  virtual IntPairs
  get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const IMP_OVERRIDE;
  using ClosePairsFinder::get_close_pairs;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis)
      const IMP_OVERRIDE;

  ParticleIndexPairs get_close_pairs(Model *m, const ParticleIndexes &pc) const;
  ParticleIndexPairs get_close_pairs(Model *m, const ParticleIndexes &pca,
                                     const ParticleIndexes &pcb) const;
  IMP_OBJECT_METHODS(GridClosePairsFinder);

};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GRID_CLOSE_PAIRS_FINDER_H */
