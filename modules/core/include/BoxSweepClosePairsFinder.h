/**
 *  \file IMP/core/BoxSweepClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H
#define IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H

#include <IMP/core/core_config.h>
#include "ClosePairsFinder.h"
#include "close_pairs_finder_macros.h"

IMPCORE_BEGIN_NAMESPACE

#if defined(IMP_DOXYGEN) || defined(IMP_CORE_USE_IMP_CGAL)
//! Find all nearby pairs by sweeping the bounding boxes
/** This method is much faster than the quadratic one when
    there are are large sets of points.

    \requires{class BoxSweepClosePairsFinder, CGAL}
    \see IMP::container::ClosePairsScoreState
*/
class IMPCOREEXPORT BoxSweepClosePairsFinder : public ClosePairsFinder {
 public:
  BoxSweepClosePairsFinder();

#ifndef SWIG
  using ClosePairsFinder::get_close_pairs;
#else
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pc) const;
  ParticlePairsTemp get_close_pairs(const ParticlesTemp &pca,
                                    const ParticlesTemp &pcb) const;
#endif
  virtual IntPairs
  get_close_pairs(const algebra::BoundingBox3Ds &bbs) const IMP_OVERRIDE;
  virtual IntPairs
  get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_close_pairs(Model *m,
                                             const ParticleIndexes &pc)
      const IMP_OVERRIDE;
  virtual ParticleIndexPairs get_close_pairs(Model *m,
                                             const ParticleIndexes &pca,
                                             const ParticleIndexes &pcb)
      const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const ParticleIndexes &pis)
      const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(BoxSweepClosePairsFinder);
};
#endif /* IMP_USE_CGAL */

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H */
