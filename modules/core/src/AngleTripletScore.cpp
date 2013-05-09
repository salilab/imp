/**
 *  \file AngleTripletScore.cpp
 *  \brief A Score on the angle between a triplet of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/AngleTripletScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>
#include <boost/tuple/tuple.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

AngleTripletScore::AngleTripletScore(UnaryFunction *f) : f_(f) {}

Float AngleTripletScore::evaluate(const ParticleTriplet &p,
                                  DerivativeAccumulator *da) const {
  IMP_CHECK_OBJECT(f_.get());
  IMP_CHECK_OBJECT(p[0]);
  IMP_CHECK_OBJECT(p[1]);
  IMP_CHECK_OBJECT(p[2]);
  XYZ d0 = XYZ::decorate_particle(p[0]);
  XYZ d1 = XYZ::decorate_particle(p[1]);
  XYZ d2 = XYZ::decorate_particle(p[2]);

  Float score;

  if (da) {
    algebra::Vector3D derv0, derv1, derv2;
    double angle = internal::angle(d0, d1, d2, &derv0, &derv1, &derv2);

    Float deriv;
    boost::tie(score, deriv) = f_->evaluate_with_derivative(angle);
    d0.add_to_derivatives(derv0 * deriv, *da);
    d1.add_to_derivatives(derv1 * deriv, *da);
    d2.add_to_derivatives(derv2 * deriv, *da);
  } else {
    double angle = internal::angle(d0, d1, d2, nullptr, nullptr, nullptr);
    score = f_->evaluate(angle);
  }
  return score;
}

void AngleTripletScore::do_show(std::ostream &out) const {
  out << "function is " << *f_;
}

IMPCORE_END_NAMESPACE
