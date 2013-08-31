/**
 *  \file IMP/score_functor/Score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SCORE_H
#define IMPSCORE_FUNCTOR_SCORE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/base_types.h>
#include <IMP/particle_index.h>
#include <IMP/base/warning_macros.h>
#include <limits>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE
/** A functor for computing a distance based score for two particles.
 */
struct Score {
  // swig gets confused otherwise
  Score() {}
#ifdef IMP_DOXYGEN
  /** Return the score at the passed feature size (eg distance). The involved
      particle indexes are passed along.

      \pre get_is_trivially_zero() or get_maximum_range() has been called
      and get_is_trivially_zero() is false. This allows things to be cached
      across those calls.
  */
  template <unsigned int D>
  double get_score(Model *m, const kernel::ParticleIndexTuple<D> &p,
                   double distance) const;
  /** Return the score and derivative at the passed feature size (eg distance).
      The derivative is for the feature decreasing.

      \pre get_is_trivially_zero() or get_maximum_range() has been called
      and get_is_trivially_zero() is false. This allows things to be cached
      across those calls.
  */
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *m,
                                          const kernel::ParticleIndexTuple<D> &p,
                                          double distance) const;
#endif
  /** Return true if the function can be easily determined to be zero at the
      passed squared distance. The default implementation provided here
      returns false.

      \note That it is squared distance, not distance.
  */
  template <unsigned int D>
  bool get_is_trivially_zero(Model *m, const base::Array<D, kernel::ParticleIndex> &p,
                             double squared_distance) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    IMP_UNUSED(squared_distance);
    return false;
  }
  /** Return an upper bound on the distance at which the score can be
      non-zero. The default implementation provided here returns infinity.*/
  template <unsigned int D>
  double get_maximum_range(Model *m,
                           const base::Array<D, kernel::ParticleIndex> &p) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    return std::numeric_limits<double>::infinity();
  }
  /** Return the set of particles read when particle p is part of the passed
      tuples. The default implementation provided here just returns the list
      containing p.*/
  ModelObjectsTemp get_inputs(Model *m, const kernel::ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
  }
  void show(std::ostream &) const {}
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SCORE_H */
