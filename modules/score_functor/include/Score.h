/**
 *  \file Score.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SCORE_H
#define IMPSCORE_FUNCTOR_SCORE_H

#include "score_functor_config.h"
#include <IMP/base_types.h>
#include <IMP/particle_index.h>
#include <IMP/base/warning_macros.h>
#include <limits>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE
/** A functor for computing a distance based score for two particles.*/
struct Score {
  // swig gets confused otherwise
  Score(){}
#ifdef IMP_DOXYGEN
  /** Return the score at the passed feature size (eg distance). The involved
      particle indexes are passed along.*/
  template <unsigned int D>
  double get_score(Model *m, const ParticleIndexTuple<D>& p,
                    double distance) const;
  /** Return the score and derivative at the passed feature size (eg distance).
      The derivative is for the feature decreasing.*/
  template <unsigned int D>
  DerivativePair get_score_and_derivative(Model *m,
                                          const ParticleIndexTuple<D>& p,
                                          double distance) const;
#endif
  /** Return true if the function can be easily determined to be zero at the
      passed squared distance.

      \note That it is squared distance, not distance.
  */
  template <unsigned int D>
  bool get_is_trivially_zero(Model *m, const ParticleIndexTuple<D>& p,
                   double squared_distance) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    IMP_UNUSED(squared_distance);
    return false;
  }
  /** Return an upper bound on the distance at which the score can be
      non-zero.*/
  template <unsigned int D>
  double get_maximum_range(Model *m, const ParticleIndexTuple<D>& p) const {
    IMP_UNUSED(m);
    IMP_UNUSED(p);
    return std::numeric_limits<double>::infinity();
  }
  ParticlesTemp get_input_particles(Particle *p) const {
    return ParticlesTemp(1,p);
  }
  ContainersTemp get_input_containers(Particle *) const {
    return ContainersTemp();
  }
  void show(std::ostream &) const{}
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_SCORE_H */
