/**
 *  \file IMP/core/DistanceToSingletonScore.h
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H
#define IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H

#include <IMP/core/core_config.h>
#include "XYZ.h"
#include "internal/evaluate_distance_pair_score.h"
#include <IMP/generic.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonScore.h>
#include <IMP/Pointer.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \htmlinclude restrain_in_sphere.py
 */
template <class UF>
class GenericDistanceToSingletonScore : public SingletonScore
{
  IMP::OwnerPointer<UF> f_;
  algebra::Vector3D pt_;
  struct StaticD
  {
    algebra::Vector3D v_;
    StaticD(algebra::Vector3D v): v_(v){}
    Float get_coordinate(unsigned int i) {return v_[i];}
    void add_to_derivatives(algebra::Vector3D v, DerivativeAccumulator){
      IMP_UNUSED(v);
      IMP_WARN( "DistanceTo dropped deriv of " <<  v << std::endl);
    }
  };
public:
  GenericDistanceToSingletonScore(UF *f,
                                  const algebra::Vector3D& pt);
  IMP_SIMPLE_SINGLETON_SCORE(GenericDistanceToSingletonScore);
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)


template <class UF>
GenericDistanceToSingletonScore<UF>
::GenericDistanceToSingletonScore(UF *f,
                                  const algebra::Vector3D &v)
    : f_(f), pt_(v){}
template <class UF>
Float GenericDistanceToSingletonScore<UF>::evaluate(Particle *b,
                                         DerivativeAccumulator *da) const
{
  Float v= internal::evaluate_distance_pair_score(XYZ(b),
                                                  StaticD(pt_), da,
                                                  f_.get(), boost::lambda::_1);
  IMP_LOG_VERBOSE( "DistanceTo from " << XYZ(b) << " to "
          << pt_ << " scored " << v << std::endl);
  return v;
}
template <class UF>
void GenericDistanceToSingletonScore<UF>::do_show(std::ostream &out) const
{
  out << "function " << *f_;
}

#endif

/** Use an IMP::UnaryFunction to score a distance to a point.*/
IMP_GENERIC_OBJECT(DistanceToSingletonScore, distance_to_singleton_score,
                   UnaryFunction,
                   (UnaryFunction *f, const algebra::Vector3D& pt),
                   (f, pt));



//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \htmlinclude restrain_in_sphere.py
 */
class IMPCOREEXPORT SphereDistanceToSingletonScore : public SingletonScore
{
  IMP::OwnerPointer<UnaryFunction> f_;
  algebra::Vector3D pt_;
  struct StaticD
  {
    algebra::Vector3D v_;
    StaticD(algebra::Vector3D v): v_(v){}
    Float get_coordinate(unsigned int i) {return v_[i];}
    void add_to_derivatives(algebra::Vector3D v, DerivativeAccumulator){
      IMP_UNUSED(v);
      IMP_LOG_VERBOSE( "DistanceTo dropped deriv of " <<  v << std::endl);
    }
  };
public:
  SphereDistanceToSingletonScore(UnaryFunction *f,
                                 const algebra::Vector3D& pt);
  IMP_SIMPLE_SINGLETON_SCORE(SphereDistanceToSingletonScore);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H */
