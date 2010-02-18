/**
 *  \file AngleTripletScore.cpp
 *  \brief A Score on the angle between a triplet of particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/core/AngleTripletScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/UnaryFunction.h>
#include <boost/tuple/tuple.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

AngleTripletScore::AngleTripletScore(UnaryFunction *f): f_(f){}

Float AngleTripletScore::evaluate(const ParticleTriplet &p,
                                  DerivativeAccumulator *da) const
{
  IMP_CHECK_OBJECT(f_.get());
  IMP_CHECK_OBJECT(p[0]);
  IMP_CHECK_OBJECT(p[1]);
  IMP_CHECK_OBJECT(p[2]);
  XYZ d0 = XYZ::decorate_particle(p[0]);
  XYZ d1 = XYZ::decorate_particle(p[1]);
  XYZ d2 = XYZ::decorate_particle(p[2]);

  algebra::Vector3D rij = d1.get_vector_to(d0);
  algebra::Vector3D rkj = d1.get_vector_to(d2);

  Float scalar_product = rij.scalar_product(rkj);
  Float mag_rij = rij.get_magnitude();
  Float mag_rkj = rkj.get_magnitude();
  Float mag_product = mag_rij * mag_rkj;

  // avoid division by zero
  Float cosangle = std::abs(mag_product) > 1e-12 ? scalar_product / mag_product
                                                 : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<Float>(1.0)),
                      static_cast<Float>(-1.0));

  Float angle = std::acos(cosangle);
  Float score;

  if (da) {
    Float deriv;
    boost::tie(score, deriv) = f_->evaluate_with_derivative(angle);

    algebra::Vector3D unit_rij = rij.get_unit_vector();
    algebra::Vector3D unit_rkj = rkj.get_unit_vector();

    Float sinangle = std::abs(std::sin(angle));

    Float fact_ij = sinangle * mag_rij;
    Float fact_kj = sinangle * mag_rkj;
    // avoid division by zero
    fact_ij = std::max(static_cast<Float>(1e-12), fact_ij);
    fact_kj = std::max(static_cast<Float>(1e-12), fact_kj);

    for (int i = 0; i < 3; ++i) {
      Float derv0 = deriv * (unit_rij[i] * cosangle - unit_rkj[i]) / fact_ij;
      Float derv2 = deriv * (unit_rkj[i] * cosangle - unit_rij[i]) / fact_kj;
      d0.add_to_derivative(i, derv0, *da);
      d1.add_to_derivative(i, -derv0 - derv2, *da);
      d2.add_to_derivative(i, derv2, *da);
    }
  } else {
    score = f_->evaluate(angle);
  }
  return score;
}

void AngleTripletScore::do_show(std::ostream &out) const
{
  out << "function is " << *f_;
}

IMPCORE_END_NAMESPACE
