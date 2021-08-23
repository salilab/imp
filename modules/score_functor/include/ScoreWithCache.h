/**
 *  \file IMP/score_functor/ScoreWithCache.h
 *  \brief A Score on the distance between a pair of particles, with cache.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SCORE_WITH_CACHE_H
#define IMPSCORE_FUNCTOR_SCORE_WITH_CACHE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/base_types.h>
#include <IMP/particle_index.h>
#include <IMP/warning_macros.h>
#include <IMP/Particle.h>
#include <limits>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! A cached functor for computing a distance based score for D particles.
/** This is similar to Score but requires callers to call check_cache_valid()
    in any scoring function evaluation before any of the methods that use
    the cache. */
struct ScoreWithCache {
  // swig gets confused otherwise
  ScoreWithCache() {}
#ifdef IMP_DOXYGEN
  //! Make sure the cache is up to date.
  /** This must be called in any scoring function evaluation before any
      of the methods that use the cache. */
  void check_cache_valid(Model *m) const;

  //! Return the score at the passed feature size (eg distance).
  /** The involved particle indexes are passed along.

      \pre get_is_trivially_zero_with_cache() or get_maximum_range() has
           been called and get_is_trivially_zero_with_cache() is false.
  */
  template <unsigned int D>
  double get_score_with_cache(Model *m, const Array<D, ParticleIndex> &p,
                              double distance) const;

  //! Return the score and derivative at the passed feature size (eg distance).
  /** The derivative is for the feature decreasing.

      \pre get_is_trivially_zero_with_cache() or get_maximum_range() has
           been called and get_is_trivially_zero_with_cache() is false.
  */
  template <unsigned int D>
  DerivativePair get_score_and_derivative_with_cache(
      Model *m, const Array<D, ParticleIndex> &p,
      double distance) const;
#endif

  /** Return true if the function can be easily determined to be zero at the
      passed squared distance. The default implementation provided here
      returns false.

      \note That it is squared distance, not distance.
  */
  template <unsigned int D>
  bool get_is_trivially_zero_with_cache(Model *m,
                             const Array<D, ParticleIndex> &p,
                             double squared_distance) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    IMP_UNUSED(squared_distance);
    return false;
  }
  /** Return an upper bound on the distance at which the score can be
      non-zero. The default implementation provided here returns infinity. */
  template <unsigned int D>
  double get_maximum_range(
      Model *m, const Array<D, ParticleIndex> &p) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    return std::numeric_limits<double>::infinity();
  }
  /** Return the set of particles read when particle p is part of the passed
      tuples. The default implementation provided here just returns the list
      containing p. */
  ModelObjectsTemp get_inputs(
      Model *m, const ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
  }
  void show(std::ostream &) const {}
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SCORE_WITH_CACHE_H */
