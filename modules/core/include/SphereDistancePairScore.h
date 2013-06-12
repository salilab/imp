/**
 *  \file IMP/core/SphereDistancePairScore.h
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include "XYZR.h"
#include <IMP/score_functor/UnaryFunctionEvaluate.h>
#include <IMP/score_functor/SphereDistance.h>
#include <IMP/score_functor/Shift.h>
#include <IMP/score_functor/Harmonic.h>
#include <IMP/score_functor/HarmonicLowerBound.h>
#include <IMP/score_functor/HarmonicUpperBound.h>
#include <IMP/score_functor/distance_pair_score_macros.h>

IMPCORE_BEGIN_NAMESPACE

// needed to keep swig from getting confused or breaking line
// lengths
typedef score_functor::SphereDistance<score_functor::UnaryFunctionEvaluate>
    SphereDistanceScore;

//! A score on the distance between the surfaces of two spheres.
/** \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see SoftSpherePairScore
 */
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(SphereDistancePairScore, SphereDistanceScore,
                                (UnaryFunction *uf,
                                 std::string name =
                                     "SphereDistancePairScore%1%"),
                                (score_functor::UnaryFunctionEvaluate(uf)));

typedef score_functor::SphereDistance<score_functor::Shift<
    score_functor::HarmonicUpperBound> > HarmonicUpperBoundSphereDistanceScore;

//!A harmonic upper bound on the distance between two spheres
/** \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
 */
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(
    HarmonicUpperBoundSphereDistancePairScore,
    HarmonicUpperBoundSphereDistanceScore,
    (double x0, double k,
     std::string name = "HarmonicUpperBoundSphereDistancePairScore%1%"),
    (score_functor::Shift<score_functor::HarmonicUpperBound>(
        x0, score_functor::HarmonicUpperBound(k))));

//!A harmonic upper bound on the diameter of the span of two spheres
/** This restraint restraints how far the furthest points of two spheres
    are from one another.

    \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT HarmonicUpperBoundSphereDiameterPairScore
    : public PairScore {
  double x0_, k_;

 public:
  HarmonicUpperBoundSphereDiameterPairScore(double d0, double k);
  double get_rest_length() const { return x0_; }
  double get_stiffness() const { return k_; }
  double evaluate_index(Model *m, const ParticleIndexPair& p,
                         DerivativeAccumulator *da) const IMP_OVERRIDE;
  ModelObjectsTemp do_get_inputs(Model *m,
                                 const ParticleIndexes &pis) const;
  IMP_PAIR_SCORE_METHODS(HarmonicUpperBoundSphereDiameterPairScore);
  IMP_OBJECT_METHODS(HarmonicUpperBoundSphereDiameterPairScore);;
};

IMP_OBJECTS(HarmonicUpperBoundSphereDiameterPairScore,
            HarmonicUpperBoundSphereDiameterPairScores);

typedef score_functor::SphereDistance<
    score_functor::Shift<score_functor::Harmonic> > HarmonicSphereDistanceScore;

//!A harmonic score on the distance between two spheres
/** \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
 */
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(HarmonicSphereDistancePairScore,
                                HarmonicSphereDistanceScore,
                                (double x0, double k,
                                 std::string name =
                                     "HarmonicSphereDistancePairScore%1%"),
                                (score_functor::Shift<score_functor::Harmonic>(
                                    x0, score_functor::Harmonic(k))));

#ifndef IMP_DOXYGEN
inline double HarmonicUpperBoundSphereDiameterPairScore::evaluate_index(
    Model *m, const ParticleIndexPair &p, DerivativeAccumulator *da) const {
  algebra::Vector3D delta =
      m->get_sphere(p[0]).get_center() - m->get_sphere(p[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance = delta.get_magnitude();
  double shifted_distance = distance - x0_ + m->get_sphere(p[0]).get_radius() +
                            m->get_sphere(p[1]).get_radius();
  if (shifted_distance < 0) return 0;
  double score = .5 * k_ * square(shifted_distance);
  if (da && distance > MIN_DISTANCE) {
    double deriv = k_ * shifted_distance;
    algebra::Vector3D uv = delta / distance;
    m->add_to_coordinate_derivatives(p[0], uv * deriv, *da);
    m->add_to_coordinate_derivatives(p[1], -uv * deriv, *da);
  }
  return score;
}
#endif

//! A score on the normalized distance between the surfaces of two spheres
/** The distance between the surfaces of the two spheres is divided by the
    smaller radius to normalize it.
    \see SphereDistancePairScore
    \see XYZR
    \see DistancePairScore
 */
class IMPCOREEXPORT NormalizedSphereDistancePairScore : public PairScore {
  IMP::base::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;

 public:
  NormalizedSphereDistancePairScore(UnaryFunction *f,
                                    FloatKey radius = FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(NormalizedSphereDistancePairScore);
};

//! A score on a weighted distance between the surfaces of two spheres
/** The distance between the surfaces of the two spheres is multiplied by
    the sum of a specific attribute
    \see SphereDistancePairScore
    \see XYZR
    \see DistancePairScore
 */
class IMPCOREEXPORT WeightedSphereDistancePairScore : public PairScore {
  IMP::base::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
  FloatKey weight_;

 public:
  WeightedSphereDistancePairScore(UnaryFunction *f, FloatKey weight,
                                  FloatKey radius = FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(WeightedSphereDistancePairScore);
};

typedef score_functor::SphereDistance<score_functor::HarmonicLowerBound>
    SoftSphereDistanceScore;

/** This class is equivalent to, but faster than a
    SphereDistancePairScore with a HarmonicLowerBound.
*/
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(SoftSpherePairScore, SoftSphereDistanceScore,
                                (double k,
                                 std::string name = "SoftSpherePairScore%1%"),
                                (score_functor::HarmonicLowerBound(k)));

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H */
