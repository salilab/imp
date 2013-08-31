/**
 *  \file IMP/core/QuadraticClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H
#define IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"
#include <IMP/core/core_config.h>

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/** \see ClosePairsScoreState
 */
class IMPCOREEXPORT QuadraticClosePairsFinder : public ClosePairsFinder {
 public:
  QuadraticClosePairsFinder();
  bool get_are_close(kernel::Particle *a, kernel::Particle *b) const {
    return get_are_close(a->get_model(), a->get_index(), b->get_index());
  }
  bool get_are_close(Model *m, kernel::ParticleIndex a, kernel::ParticleIndex b) const;
#ifndef SWIG
  using ClosePairsFinder::get_close_pairs;
#else
  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlesTemp &pc) const;
  kernel::ParticlePairsTemp get_close_pairs(const kernel::ParticlesTemp &pca,
                                    const kernel::ParticlesTemp &pcb) const;
#endif
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &bas,
                                   const algebra::BoundingBox3Ds &bbs) const
      IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_close_pairs(Model *m,
                                             const kernel::ParticleIndexes &pc) const
      IMP_OVERRIDE;
  virtual kernel::ParticleIndexPairs get_close_pairs(Model *m,
                                             const kernel::ParticleIndexes &pca,
                                             const kernel::ParticleIndexes &pcb) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;

  IMP_OBJECT_METHODS(QuadraticClosePairsFinder);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H */
