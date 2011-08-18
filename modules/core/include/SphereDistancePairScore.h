/**
 *  \file SphereDistancePairScore.h
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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


//!A harmonic upper bound on the distance between two spheres
/** \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT HarmonicUpperBoundSphereDistancePairScore : public PairScore
{
  double x0_, k_;
public:
  HarmonicUpperBoundSphereDistancePairScore(double d0, double k);
  double get_rest_length() const {
    return x0_;
  }
  double get_stiffness() const {
    return k_;
  }
  IMP_INDEX_PAIR_SCORE(HarmonicUpperBoundSphereDistancePairScore);
};

IMP_OBJECTS(HarmonicUpperBoundSphereDistancePairScore,
            HarmonicUpperBoundSphereDistancePairScores);



//!A harmonic upper bound on the diameter of the span of two spheres
/** This restraint restraints how far the furthest points of two spheres
    are from one another.

    \see XYZR
    \see SpherePairScore
    \see SoftSpherePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT HarmonicUpperBoundSphereDiameterPairScore : public PairScore
{
  double x0_, k_;
public:
  HarmonicUpperBoundSphereDiameterPairScore(double d0, double k);
  double get_rest_length() const {
    return x0_;
  }
  double get_stiffness() const {
    return k_;
  }
  IMP_INDEX_PAIR_SCORE(HarmonicUpperBoundSphereDiameterPairScore);
};

IMP_OBJECTS(HarmonicUpperBoundSphereDiameterPairScore,
            HarmonicUpperBoundSphereDiameterPairScores);



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
  IMP_INDEX_PAIR_SCORE(HarmonicSphereDistancePairScore);
};

IMP_OBJECTS(HarmonicSphereDistancePairScore, HarmonicSphereDistancePairScores);





#ifndef IMP_DOXYGEN
inline double HarmonicSphereDistancePairScore::evaluate_index(Model *m,
                                const ParticleIndexPair& p,
           DerivativeAccumulator *da) const {
  algebra::Vector3D delta=m->get_sphere(p[0]).get_center()
    - m->get_sphere(p[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- x0_
    - m->get_sphere(p[0]).get_radius()
    - m->get_sphere(p[1]).get_radius();
  double score= .5*k_*square(shifted_distance);
  if (da && distance > MIN_DISTANCE) {
    double deriv= k_*shifted_distance;
    algebra::Vector3D uv= delta/distance;
    m->add_to_coordinate_derivatives(p[0], uv*deriv, *da);
    m->add_to_coordinate_derivatives(p[1], -uv*deriv, *da);
  }
  return score;
}

inline double
HarmonicUpperBoundSphereDistancePairScore::evaluate_index(Model *m,
                                  const ParticleIndexPair& p,
           DerivativeAccumulator *da) const {
  algebra::Vector3D delta=m->get_sphere(p[0]).get_center()
    - m->get_sphere(p[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance= delta.get_magnitude();
  double shifted_distance = distance- x0_
    - m->get_sphere(p[0]).get_radius()
    - m->get_sphere(p[1]).get_radius();
  if (shifted_distance < 0) return 0;
  double score= .5*k_*square(shifted_distance);
  if (da && distance > MIN_DISTANCE) {
    double deriv= k_*shifted_distance;
    algebra::Vector3D uv= delta/distance;
    m->add_to_coordinate_derivatives(p[0], uv*deriv, *da);
    m->add_to_coordinate_derivatives(p[1], -uv*deriv, *da);
  }
  return score;
}


inline double
HarmonicUpperBoundSphereDiameterPairScore::evaluate_index(Model *m,
                                  const ParticleIndexPair& p,
           DerivativeAccumulator *da) const {
  algebra::Vector3D delta=m->get_sphere(p[0]).get_center()
    - m->get_sphere(p[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance= delta.get_magnitude();
  double shifted_distance = distance- x0_
    + m->get_sphere(p[0]).get_radius()
    + m->get_sphere(p[1]).get_radius();
  if (shifted_distance < 0) return 0;
  double score= .5*k_*square(shifted_distance);
  if (da && distance > MIN_DISTANCE) {
    double deriv= k_*shifted_distance;
    algebra::Vector3D uv= delta/distance;
    m->add_to_coordinate_derivatives(p[0], uv*deriv, *da);
    m->add_to_coordinate_derivatives(p[1], -uv*deriv, *da);
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
  IMP_INDEX_PAIR_SCORE(SoftSpherePairScore);
};


IMP_OBJECTS(SoftSpherePairScore, SoftSpherePairScores);


#ifndef IMP_DOXYGEN
inline double SoftSpherePairScore
::evaluate_index(Model *m, const ParticleIndexPair& pp,
           DerivativeAccumulator *da) const {
  algebra::Vector3D delta=m->get_sphere(pp[0]).get_center()
    - m->get_sphere(pp[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double rs= m->get_sphere(pp[0]).get_radius()
    + m->get_sphere(pp[1]).get_radius();
  if (distance2 > square(rs)) return 0;
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- rs;
  double deriv= k_*shifted_distance;
  double score= .5*deriv*shifted_distance;
  if (da && distance > MIN_DISTANCE) {
    algebra::Vector3D uv= delta/distance;
    m->add_to_coordinate_derivatives(pp[0], uv*deriv, *da);
    m->add_to_coordinate_derivatives(pp[1], -uv*deriv, *da);
  }
  return score;
}



#endif


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H */
