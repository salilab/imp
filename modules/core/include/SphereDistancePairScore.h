/**
 *  \file SphereDistancePairScore.h
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H

#include "core_config.h"
#include "XYZR.h"
#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! A score on the distance between the surfaces of two spheres.
/** \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see SoftSpherePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT SphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
public:
  SphereDistancePairScore(UnaryFunction *f,
                          FloatKey radius=FloatKey("radius"));
  UnaryFunction* get_unary_function() const {
    return f_;
  }
  IMP_SIMPLE_PAIR_SCORE(SphereDistancePairScore);
};


//!A harmonic score on the distance between two spheres
/** \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT HarmonicSphereDistancePairScore : public PairScore
{
  double x0_, k_;
public:
  HarmonicSphereDistancePairScore(double d0, double k);
  double get_rest_length() const {
    return x0_;
  }
  double get_stiffness() const {
    return k_;
  }
  IMP_SIMPLE_PAIR_SCORE(HarmonicSphereDistancePairScore);
};

IMP_OBJECTS(HarmonicSphereDistancePairScore, HarmonicSphereDistancePairScores);



#ifndef IMP_DOXYGEN
inline double HarmonicSphereDistancePairScore::evaluate(const ParticlePair &p,
                            DerivativeAccumulator *da) const {
  XYZR d0(p[0]), d1(p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- x0_- d0.get_radius() - d1.get_radius();
  double score= .5*k_*square(shifted_distance);
  if (!da || distance < MIN_DISTANCE) return score;
  double deriv= k_*shifted_distance;
  algebra::Vector3D uv= delta/distance;
  if (da) {
    d0.add_to_derivatives(uv*deriv, *da);
    d1.add_to_derivatives(-uv*deriv, *da);
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
class IMPCOREEXPORT NormalizedSphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
public:
  NormalizedSphereDistancePairScore(UnaryFunction *f,
                                    FloatKey radius=FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(NormalizedSphereDistancePairScore);
};


//! A score on a weighted distance between the surfaces of two spheres
/** The distance between the surfaces of the two spheres is multiplied by
    the sum of a specific attribute
    \see SphereDistancePairScore
    \see XYZR
    \see DistancePairScore
 */
class IMPCOREEXPORT WeightedSphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
  FloatKey weight_;
public:
  WeightedSphereDistancePairScore(UnaryFunction *f,
               FloatKey weight,
               FloatKey radius=FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(WeightedSphereDistancePairScore);
};


/** This class is equivalent to, but faster than a
    SphereDistancePairScore with a HarmonicLowerBound.
*/
class IMPCOREEXPORT SoftSpherePairScore: public PairScore {
  double k_;
public:
  SoftSpherePairScore(double k): k_(k){}
  IMP_SIMPLE_PAIR_SCORE(SoftSpherePairScore);
};


IMP_OBJECTS(SoftSpherePairScore, SoftSpherePairScores);


#ifndef IMP_DOXYGEN
inline double SoftSpherePairScore::evaluate(const ParticlePair &p,
                                     DerivativeAccumulator *da) const {
  XYZR d0(p[0]), d1(p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  if (distance2 > square(d0.get_radius()+d1.get_radius())) return 0;
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- d0.get_radius()-d1.get_radius();
  double deriv= k_*shifted_distance;
  double score= .5*deriv*shifted_distance;
  if (!da || distance < MIN_DISTANCE) return score;
  algebra::Vector3D uv= delta/distance;
  if (da) {
    d0.add_to_derivatives(uv*deriv, *da);
    d1.add_to_derivatives(-uv*deriv, *da);
  }

  return score;
}
#endif


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H */
