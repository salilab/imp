/**
 *  \file DihedralRestraint.cpp \brief Dihedral restraint between four
 *                                     particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/DihedralRestraint.h>
#include <IMP/core/XYZDecorator.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>
#include <IMP/Vector3D.h>

#include <boost/tuple/tuple.hpp>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

DihedralRestraint::DihedralRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2, Particle* p3,
                                     Particle* p4)
{
  p_[0]=p1;
  p_[1]=p2;
  p_[2]=p3;
  p_[3]=p4;

  score_func_ = score_func;
}


//! Destructor
DihedralRestraint::~DihedralRestraint()
{
}


//! Calculate the score for this dihedral restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float DihedralRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(score_func_);
  XYZDecorator d0 = XYZDecorator::cast(p_[0]);
  XYZDecorator d1 = XYZDecorator::cast(p_[1]);
  XYZDecorator d2 = XYZDecorator::cast(p_[2]);
  XYZDecorator d3 = XYZDecorator::cast(p_[3]);

  Vector3D rij = d1.get_vector_to(d0);
  Vector3D rkj = d1.get_vector_to(d2);
  Vector3D rkl = d3.get_vector_to(d2);

  Vector3D v1 = rij.vector_product(rkj);
  Vector3D v2 = rkj.vector_product(rkl);
  Float scalar_product = v1.scalar_product(v2);
  Float mag_product = v1.get_magnitude() * v2.get_magnitude();

  // avoid division by zero
  Float cosangle = std::abs(mag_product) > 1e-12
                   ? scalar_product / std::sqrt(mag_product) : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<Float>(1.0)),
                      static_cast<Float>(-1.0));

  Float angle = std::acos(cosangle);
  // get sign
  Vector3D v0 = v1.vector_product(v2);
  Float sign = rkj.scalar_product(v0);
  if (sign < 0.0) {
    angle = -angle;
  }

  Float score;

  if (accum) {
    Float deriv;
    boost::tie(score, deriv) = score_func_->evaluate_with_derivative(angle);

    // method for derivative calculation from van Schaik et al.
    // J. Mol. Biol. 234, 751-762 (1993)
    Vector3D vijkj = rij.vector_product(rkj);
    Vector3D vkjkl = rkj.vector_product(rkl);
    Float sijkj2 = vijkj.get_squared_magnitude();
    Float skjkl2 = vkjkl.get_squared_magnitude();
    Float skj = rkj.get_magnitude();
    Float rijkj = rij.scalar_product(rkj);
    Float rkjkl = rkj.scalar_product(rkl);

    Float fact0 = sijkj2 > 1e-8 ? skj / sijkj2 : 0.0;
    Float fact1 = skj > 1e-8 ? rijkj / (skj * skj) : 0.0;
    Float fact2 = skj > 1e-8 ? rkjkl / (skj * skj) : 0.0;
    Float fact3 = skjkl2 > 1e-8 ? -skj / skjkl2 : 0.0;

    for (int i = 0; i < 3; ++i) {
      Float derv0 = deriv * fact0 * vijkj[i];
      Float derv3 = deriv * fact3 * vkjkl[i];
      Float derv1 = (fact1 - 1.0) * derv0 - fact2 * derv3;
      Float derv2 = (fact2 - 1.0) * derv3 - fact1 * derv0;

      d0.add_to_coordinate_derivative(i, derv0, *accum);
      d1.add_to_coordinate_derivative(i, derv1, *accum);
      d2.add_to_coordinate_derivative(i, derv2, *accum);
      d3.add_to_coordinate_derivative(i, derv3, *accum);
    }
  } else {
    score = score_func_->evaluate(angle);
  }
  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void DihedralRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "dihedral restraint (active):" << std::endl;
  } else {
    out << "dihedral restraint (inactive):" << std::endl;
  }

  get_version_info().show(out);
  out << "  particles: " << p_[0]->get_index();
  out << ", " << p_[1]->get_index();
  out << ", " << p_[2]->get_index();
  out << " and " << p_[3]->get_index();
  out << "  ";
  score_func_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
