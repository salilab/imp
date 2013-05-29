/**
 *  \file IMP/score_functor/DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_H
#define IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Create efficient distance-based pair scores.
/** This class allows one to create efficient distance-based
    pair scores in C++ by simply writing a pair of small
    functors (DistanceScore and DistanceDerivative) that do
    the scoring.

    \see DistanceScore

    \note we can add arguments to get the list of input
    particles and containers later, for now the former
    is assumed to just be the input and the latter empty.
*/
template <class DistanceScoreT> class DistancePairScore : public PairScore {
  DistanceScoreT ds_;

 public:
  typedef DistanceScoreT DistanceScore;
  // for backwards compat
  DistancePairScore(const DistanceScore &t0,
                    std::string name = "FunctorDistancePairScore %1%")
      : PairScore(name), ds_(t0) {}

  IMP_INDEX_PAIR_SCORE(DistancePairScore);
};

#ifndef IMP_DOXYGEN
template <class DistanceScore>
inline double DistancePairScore<DistanceScore>::evaluate_index(
    Model *m, const ParticleIndexPair &p, DerivativeAccumulator *da) const {
  algebra::Vector3D delta =
      m->get_sphere(p[0]).get_center() - m->get_sphere(p[1]).get_center();
  double sq = delta.get_squared_magnitude();
  if (ds_.get_is_trivially_zero(m, p, sq)) {
    return 0;
  }
  double dist = std::sqrt(sq);
  if (da) {
    std::pair<double, double> sp = ds_.get_score_and_derivative(m, p, dist);
    static const double MIN_DISTANCE = .00001;
    algebra::Vector3D uv;
    if (dist > MIN_DISTANCE) {
      uv = delta / dist;
    } else {
      uv = algebra::get_zero_vector_d<3>();
    }
    m->add_to_coordinate_derivatives(p[0], uv * sp.second, *da);
    m->add_to_coordinate_derivatives(p[1], -uv * sp.second, *da);
    return sp.first;
  } else {
    return ds_.get_score(m, p, dist);
  }
}
template <class DistanceScore>
inline ModelObjectsTemp DistancePairScore<DistanceScore>::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += ds_.get_inputs(m, pis);
  return ret;
}
template <class DistanceScore>
inline void DistancePairScore<DistanceScore>::do_show(std::ostream &) const {}
#endif

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_H */
