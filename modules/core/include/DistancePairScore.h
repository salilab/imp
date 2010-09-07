/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_DISTANCE_PAIR_SCORE_H

#include "core_config.h"
#include "XYZ.h"
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance between two particles.
/** \see SphereDistancePairScore
    \see HarmonicDistancePairScore
*/
class IMPCOREEXPORT DistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  IMP_SIMPLE_PAIR_SCORE(DistancePairScore);
};

IMP_OBJECTS(DistancePairScore, DistancePairScores);

/** This class is significantly faster than using a DistancePairScore
    with a Harmonic.
*/
class IMPCOREEXPORT HarmonicDistancePairScore: public PairScore
{
  const double x0_, k_;
public:
  HarmonicDistancePairScore(double x0, double k):  x0_(x0), k_(k){}
  double get_rest_length() const {
    return x0_;
  }
  double get_k() const {
    return k_;
  }
  IMP_SIMPLE_PAIR_SCORE(HarmonicDistancePairScore);
};

IMP_OBJECTS(HarmonicDistancePairScore, HarmonicDistancePairScores);

#ifndef IMP_DOXYGEN
inline double HarmonicDistancePairScore::evaluate(const ParticlePair &p,
                            DerivativeAccumulator *da) const {
  XYZ d0(p[0]), d1(p[1]);
  algebra::VectorD<3> delta;
  for (int i = 0; i < 3; ++i) {
    delta[i] = d0.get_coordinate(i) - d1.get_coordinate(i);
  }
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- x0_;
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

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_PAIR_SCORE_H */
