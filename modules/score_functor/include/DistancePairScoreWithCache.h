/**
 *  \file IMP/score_functor/DistancePairScoreWithCache.h
 *  \brief A Score on the distance between a pair of particles, using cache.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_WITH_CACHE_H
#define IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_WITH_CACHE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Create efficient distance-based pair scores, with cache.
/** This class allows one to create efficient distance-based
    pair scores in C++ by simply writing a functor (ScoreWithCache) that
    does the scoring. This is like DistancePairScore but it allows the
    functor to do once-per-model-evaluation cache updates.

    \see ScoreWithCache, DistancePairScore
*/
template <class DistanceScoreT>
class DistancePairScoreWithCache : public PairScore {
  DistanceScoreT ds_;

  double evaluate_index_with_cache(Model *m,
                                   const ParticleIndexPair &pip,
                                   DerivativeAccumulator *da) const;

 public:
  typedef DistanceScoreT DistanceScore;

  // for backwards compat
  DistancePairScoreWithCache(const DistanceScore &t0,
                    std::string name = "FunctorDistancePairScoreWithCache %1%")
      : PairScore(name), ds_(t0) {}

  virtual double evaluate_index(Model *m,
                                const ParticleIndexPair &pip,
                                DerivativeAccumulator *da) const override;

  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;

  /**
      return a reference to the functor that is applied on a pair of particles
      in order to compute their distances
  */
  DistanceScoreT& get_score_functor()
    {return ds_; }

  virtual double evaluate_indexes(
       Model *m, const ParticleIndexPairs &p,
       DerivativeAccumulator *da, unsigned int lower_bound,
       unsigned int upper_bound) const override;

  virtual double evaluate_indexes_scores(
       Model *m, const ParticleIndexPairs &p, DerivativeAccumulator *da,
       unsigned int lower_bound, unsigned int upper_bound,
       std::vector<double> &score) const override;

  virtual double evaluate_indexes_delta(
       Model *m, const ParticleIndexPairs &p, DerivativeAccumulator *da,
       const std::vector<unsigned> &indexes,
       std::vector<double> &score) const override;

  IMP_OBJECT_METHODS(DistancePairScoreWithCache);
};

#ifndef IMP_DOXYGEN
template <class DistanceScore>
inline double DistancePairScoreWithCache<DistanceScore>::evaluate_indexes(
       Model *m, const ParticleIndexPairs &p,
       DerivativeAccumulator *da, unsigned int lower_bound,
       unsigned int upper_bound) const {
  double ret = 0;
  ds_.check_cache_valid(m);
  for (unsigned int i = lower_bound; i < upper_bound; ++i) {
    ret += evaluate_index_with_cache(m, p[i], da);
  }
  return ret;
}

template <class DistanceScore>
inline double
DistancePairScoreWithCache<DistanceScore>::evaluate_indexes_scores(
       Model *m, const ParticleIndexPairs &p, DerivativeAccumulator *da,
       unsigned int lower_bound, unsigned int upper_bound,
       std::vector<double> &score) const {
  double ret = 0;
  ds_.check_cache_valid(m);
  for (unsigned int i = lower_bound; i < upper_bound; ++i) {
    double s = evaluate_index_with_cache(m, p[i], da);
    score[i] = s;
    ret += s;
  }
  return ret;
}

template <class DistanceScore>
inline double
DistancePairScoreWithCache<DistanceScore>::evaluate_indexes_delta(
       Model *m, const ParticleIndexPairs &p, DerivativeAccumulator *da,
       const std::vector<unsigned> &indexes, std::vector<double> &score) const {
  double ret = 0;
  ds_.check_cache_valid(m);
  for (unsigned it : indexes) {
    double s = evaluate_index_with_cache(m, p[it], da);
    ret = ret - score[it] + s;
    score[it] = s;
  }
  return ret;
}

template <class DistanceScore>
inline double DistancePairScoreWithCache<DistanceScore>::evaluate_index(
    Model *m, const ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  ds_.check_cache_valid(m);
  return evaluate_index_with_cache(m, p, da);
}

template <class DistanceScore>
inline double
DistancePairScoreWithCache<DistanceScore>::evaluate_index_with_cache(
    Model *m, const ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  algebra::Vector3D delta =
      m->get_sphere(std::get<0>(p)).get_center()
      - m->get_sphere(std::get<1>(p)).get_center();
  double sq = delta.get_squared_magnitude();
  if (ds_.get_is_trivially_zero_with_cache(m, p, sq)) {
    return 0;
  }
  double dist = std::sqrt(sq);
  if (da) {
    std::pair<double, double> sp = ds_.get_score_and_derivative_with_cache(
                    m, p, dist);
    static const double MIN_DISTANCE = .00001;
    algebra::Vector3D uv;
    if (dist > MIN_DISTANCE) {
      uv = delta / dist;
    } else {
      uv = algebra::get_zero_vector_d<3>();
    }
    m->add_to_coordinate_derivatives(std::get<0>(p), uv * sp.second, *da);
    m->add_to_coordinate_derivatives(std::get<1>(p), -uv * sp.second, *da);
    return sp.first;
  } else {
    return ds_.get_score_with_cache(m, p, dist);
  }
}

template <class DistanceScore>
inline ModelObjectsTemp
DistancePairScoreWithCache<DistanceScore>::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += ds_.get_inputs(m, pis);
  return ret;
}
#endif

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_DISTANCE_PAIR_SCORE_WITH_CACHE_H */
