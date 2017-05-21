/**
 *  \file IMP/score_functor/SurfaceDistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_H
#define IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/score_functor/internal/surface_helpers.h>
#include <IMP/score_functor/internal/direction_helpers.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

//! Create efficient surface distance-based pair scores.
/** This class allows one to create efficient distance-based
    pair scores between a surface and a point in C++ by simply
    writing a functor (Score) that does the scoring.

    \note In the passed `ParticleIndexPair`, the first particle must
    be for the surface.

    \see SurfaceDistancePairScore
    \see DistancePairScore
    \see Score
*/
template <class DistanceScoreT>
class SurfaceDistancePairScore : public PairScore {
  private:
    DistanceScoreT ds_;

    //! Get the distance from the surface to the point.
    virtual double get_distance(const algebra::Vector3D &center,
                                const algebra::Vector3D &normal,
                                const algebra::Vector3D &point,
                                algebra::Vector3D *delta) const {
      return internal::get_distance_from_surface(center, normal, point, delta);
    }

  public:
    typedef DistanceScoreT DistanceScore;

    SurfaceDistancePairScore(
        const DistanceScore &t0,
        std::string name = "FunctorSurfaceDistancePairScore %1%")
        : PairScore(name), ds_(t0) {}

    //! Compute the score and the derivative if needed
    /** \note In the passed `ParticleIndexPair`, the first particle must
        be for the surface.
     */
    virtual double evaluate_index(Model *m,
                                  const ParticleIndexPair &pip,
                                  DerivativeAccumulator *da) const IMP_OVERRIDE;

    virtual ModelObjectsTemp do_get_inputs(
        Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;

    DistanceScoreT& get_score_functor()
      { return ds_; }

    IMP_PAIR_SCORE_METHODS(SurfaceDistancePairScore);
    IMP_OBJECT_METHODS(SurfaceDistancePairScore);
};

#ifndef IMP_DOXYGEN
template <class DistanceScore>
inline double SurfaceDistancePairScore<DistanceScore>::evaluate_index(
    Model *m, const ParticleIndexPair &p,
    DerivativeAccumulator *da) const {
  algebra::Vector3D delta; // normal vector from surface to point

  double dist = get_distance(m->get_sphere(p[0]).get_center(),
                             internal::get_direction(m, p[0]),
                             m->get_sphere(p[1]).get_center(), &delta);

  // Using squared distance for trivial check currently doesn't work for surfaces
  // if (ds_.get_is_trivially_zero(m, p, dist * dist)) {
  //   return 0;
  // }

  if (da) {
    std::pair<double, double> sp = ds_.get_score_and_derivative(m, p, dist);
    m->add_to_coordinate_derivatives(p[0], -delta * sp.second, *da);
    m->add_to_coordinate_derivatives(p[1], delta * sp.second, *da);
    return sp.first;
  } else {
    return ds_.get_score(m, p, dist);
  }
}

template <class DistanceScore>
inline ModelObjectsTemp SurfaceDistancePairScore<
        DistanceScore>::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += ds_.get_inputs(m, pis);
  return ret;
}
#endif


//! Create efficient surface height-based pair scores.
/** This class allows one to create efficient height-based
    pair scores between a surface and a point in C++ by simply
    writing a functor (Score) that does the scoring.

    \note In the passed `ParticleIndexPair`, the first particle must
    be for the surface.

    \see SurfaceDistancePairScore
    \see DistancePairScore
    \see Score
*/
#if defined(SWIG) || defined(IMP_DOXYGEN)
template <class DistanceScoreT>
class SurfaceHeightPairScore : public PairScore {
  public:
    SurfaceHeightPairScore(
        const DistanceScore &t0,
        std::string name = "FunctorSurfaceHeightPairScore %1%");
};
#else
template <class DistanceScore>
class SurfaceHeightPairScore : public SurfaceDistancePairScore<
        DistanceScore> {
    virtual double get_distance(const algebra::Vector3D &center,
                                const algebra::Vector3D &normal,
                                const algebra::Vector3D &point,
                                algebra::Vector3D *delta) const IMP_OVERRIDE {
      return internal::get_height_above_surface(center, normal, point, delta);
    }
  public:
    SurfaceHeightPairScore(
        const DistanceScore &t0,
        std::string name = "FunctorSurfaceHeightPairScore %1%")
      : SurfaceDistancePairScore<DistanceScore>(t0, name) {}
};
#endif


//! Create efficient surface depth-based pair scores.
/** This class allows one to create efficient depth-based
    pair scores between a surface and a point in C++ by simply
    writing a functor (Score) that does the scoring.

    \note In the passed `ParticleIndexPair`, the first particle must
    be for the surface.

    \see SurfaceDistancePairScore
    \see DistancePairScore
    \see Score
*/
#if defined(SWIG) || defined(IMP_DOXYGEN)
template <class DistanceScoreT>
class SurfaceDepthPairScore : public PairScore {
  public:
    SurfaceDepthPairScore(
        const DistanceScore &t0,
        std::string name = "FunctorSurfaceDepthPairScore %1%");
};
#else
template <class DistanceScore>
class SurfaceDepthPairScore : public SurfaceDistancePairScore<
        DistanceScore> {
    virtual double get_distance(const algebra::Vector3D &center,
                                const algebra::Vector3D &normal,
                                const algebra::Vector3D &point,
                                algebra::Vector3D *delta) const IMP_OVERRIDE {
      return internal::get_depth_below_surface(center, normal, point, delta);
    }
  public:
    SurfaceDepthPairScore(
        const DistanceScore &t0,
        std::string name = "FunctorSurfaceDepthPairScore %1%")
      : SurfaceDistancePairScore<DistanceScore>(t0, name) {}
};
#endif

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE_H */
