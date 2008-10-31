/**
 *  \file BoxSweepClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H
#define IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by sweeping the bounding boxes
/** This method is much faster than the quadratic one when
    there are are large sets of points.

    \note This method requires CGAL to work.
 */
class IMPCOREEXPORT BoxSweepClosePairsFinder : public ClosePairsFinder
{
 public:
  BoxSweepClosePairsFinder();
  ~BoxSweepClosePairsFinder();

  void add_close_pairs(ParticleContainer *pc,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListParticlePairContainer *out) const;

  void add_close_pairs(ParticleContainer *pca,
                       ParticleContainer *pcb,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListParticlePairContainer *out) const;

  //! Return true if the needed libraries have been found.
  static bool get_is_implemented();
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOX_SWEEP_CLOSE_PAIRS_FINDER_H */
