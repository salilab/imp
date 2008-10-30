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
  QuadraticClosePairsFinder();
  ~QuadraticClosePairsFinder();

  void add_close_pairs(ParticleContainer *pc,
                       FilteredListParticlePairContainer *out);

  void add_close_pairs(ParticleContainer *pca,
                       ParticleContainer *pcb,
                       FilteredListParticlePairContainer *out);

  bool get_are_close(Particle *a, Particle *b) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_QUADRATIC_CLOSE_PAIRS_FINDER_H */
