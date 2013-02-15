/**
 *  \file IMP/core/BoundingBox3DSingletonScore.h
 *  \brief Score particles based on a bounding box
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H
#define IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/BoundingBoxD.h>
#include "XYZ.h"
#include "internal/evaluate_distance_pair_score.h"
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Score particles based on how far outside a box they are.
/** The radius of the particle is ignored, only the center coordinates
    are used. A particle that is contained within the bounding box has
    a score of 0. The UnaryFunction passed should return 0 when given
    a feature size of 0 and a positive value when the feature is positive.
 */
template <class UF>
class GenericBoundingBox3DSingletonScore: public SingletonScore
{
  IMP::OwnerPointer<UF> f_;
  algebra::BoundingBoxD<3> bb_;
public:
  GenericBoundingBox3DSingletonScore(UF *f,
                            const algebra::BoundingBoxD<3> &bb);

  IMP_SIMPLE_SINGLETON_SCORE(GenericBoundingBox3DSingletonScore);
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class UF>
GenericBoundingBox3DSingletonScore<UF>
::GenericBoundingBox3DSingletonScore(UF *f,
                            const algebra::BoundingBox3D &bb ): f_(f), bb_(bb){
  IMP_USAGE_CHECK(std::abs(f_->evaluate(0)) <.1,
                  "The unary function should return "
            " 0 when passed a value of 0. Not " << f_->evaluate(0));
}
template <class UF>
double GenericBoundingBox3DSingletonScore<UF>::evaluate(Particle *p,
                                           DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  core::XYZ d(p);
  algebra::Vector3D cp;
  bool outside=false;
  for (unsigned int i=0; i< 3; ++i) {
    if (bb_.get_corner(0)[i] > d.get_coordinate(i)) {
      cp[i]=bb_.get_corner(0)[i];
      outside=true;
    } else if (bb_.get_corner(1)[i] < d.get_coordinate(i)) {
      cp[i]=bb_.get_corner(1)[i];
      outside=true;
    } else {
      cp[i]= d.get_coordinate(i);
    }
  }
  if (outside) {
    IMP_LOG_VERBOSE( "Particle " << Showable(p) << " is outside box: "
            << d << " of " << bb_ << std::endl);
    algebra::Vector3D deriv;
    double v= internal::compute_distance_pair_score(d.get_coordinates()-cp,
                                                    f_.get(),&deriv,
                                                    boost::lambda::_1);
    if (da) {
      d.add_to_derivatives(deriv, *da);
    }
    return v;
  } else {
    return 0;
  }
}
template <class UF>
void GenericBoundingBox3DSingletonScore<UF>::do_show(std::ostream &out) const {
  out << "box is " << bb_ << std::endl;
}



#endif

IMP_GENERIC_OBJECT(BoundingBox3DSingletonScore, bounding_box_3d_singleton_score,
                   UnaryFunction, (UnaryFunction *f,
                                   const algebra::BoundingBoxD<3> &bb),
                   (f, bb));



IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H */
