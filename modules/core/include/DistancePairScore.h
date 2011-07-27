/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
  UnaryFunction *get_unary_function() const {return f_;}
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
  HarmonicDistancePairScore(double x0, double k);
  double get_rest_length() const {
    return x0_;
  }
  double get_stiffness() const {
    return k_;
  }
  IMP_INDEX_PAIR_SCORE(HarmonicDistancePairScore);
};

IMP_OBJECTS(HarmonicDistancePairScore, HarmonicDistancePairScores);


#ifndef IMP_DOXYGEN
inline double HarmonicDistancePairScore
::evaluate_index(Model *m,
                 const ParticleIndexPair &p,
                 DerivativeAccumulator *da) const {
  algebra::VectorD<3> delta=m->get_sphere(p[0]).get_center()
    - m->get_sphere(p[1]).get_center();
  static const double MIN_DISTANCE = .00001;
  double distance2= delta.get_squared_magnitude();
  double distance=std::sqrt(distance2);
  double shifted_distance = distance- x0_;
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

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_PAIR_SCORE_H */
