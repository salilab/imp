/**
 *  \file QuadraticClosePairsFinder.h
 *  \brief Test all pairs of particles to find close pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H
#define IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H

#include "ClosePairsFinder.h"

IMPCORE_BEGIN_NAMESPACE

//! Find all nearby pairs by testing all pairs
/**
 */
class IMPCOREEXPORT QuadraticClosePairsFinder : public ClosePairsFinder
{
 public:
  /** */
  QuadraticClosePairsFinder();
  ~QuadraticClosePairsFinder();

  void add_close_pairs(SingletonContainer *pc,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListPairContainer *out) const;

  void add_close_pairs(SingletonContainer *pca,
                       SingletonContainer *pcb,
                       Float distance,
                       FloatKey radius_key,
                       FilteredListPairContainer *out) const;
  //! Return true if the two particles are closer than distance.
  bool get_are_close(Particle *a, Particle *b,
                     Float distance,
                     FloatKey radius_key) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H */
