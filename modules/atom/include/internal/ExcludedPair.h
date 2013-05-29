/**
 *  \file ExcludedPair.h
 *  \brief A pair of Particles, used for exclusions.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_INTERNAL_EXCLUDED_PAIR_H
#define IMPATOM_INTERNAL_EXCLUDED_PAIR_H

#include <IMP/atom/atom_config.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! An unordered pair of Particles, used for exclusions.
/** Note that for performance the Particle* are not refcounted; the excluded
    pair PairFilter must keep the refcounts itself.
    Unlike a ParticlePair, the Particle* are stored sorted, so that
    ExcludedPair(a, b) == ExcludedPair(b, a).
 */
class ExcludedPair {
 public:
  Particle *a_, *b_;
  ExcludedPair(Particle *a, Particle *b) : a_(a), b_(b) {
    if (b_ < a_) {
      std::swap(a_, b_);
    }
  }
  inline bool operator<(const ExcludedPair &other) const {
    return a_ < other.a_ || (!(other.a_ < a_) && b_ < other.b_);
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_EXCLUDED_PAIR_H */
