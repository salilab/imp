/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/log.h"
#include "IMP/Vector3D.h"
#include "IMP/restraints/AngleRestraint.h"
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

AngleRestraint::AngleRestraint(Particle* p1, Particle* p2, Particle* p3,
                               ScoreFunc* score_func)
{
  add_particle(p1);
  add_particle(p2);
  add_particle(p3);

  score_func_ = score_func;
}


//! Destructor
AngleRestraint::~AngleRestraint()
{
  delete score_func_;
}


//! Calculate the score for this angle restraint.
/** \param[in] accum If not NULL, use this object to accumulate partial first
                     derivatives.
    \return Current score.
 */
Float AngleRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(score_func_);
  XYZDecorator d0 = XYZDecorator::cast(get_particle(0));
  XYZDecorator d1 = XYZDecorator::cast(get_particle(1));
  XYZDecorator d2 = XYZDecorator::cast(get_particle(2));

  Vector3D rij = d1.get_vector_to(d0);
  Vector3D rkj = d1.get_vector_to(d2);

  Float scalar_product = rij.scalar_product(rkj);
  Float mag_rij = rij.magnitude();
  Float mag_rkj = rkj.magnitude();
  Float mag_product = mag_rij * mag_rkj;

  // avoid division by zero
  Float cosangle = std::abs(mag_product) > 1e-12 ? scalar_product / mag_product
                                                 : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<Float>(1.0)),
                      static_cast<Float>(-1.0));

  Float angle = std::acos(cosangle);
  Float score;

  if (accum) {
    Float deriv;
    score = (*score_func_)(angle, deriv);

    Vector3D unit_rij = rij.get_unit_vector();
    Vector3D unit_rkj = rkj.get_unit_vector();

    Float sinangle = std::abs(std::sin(angle));

    Float fact_ij = sinangle * mag_rij;
    Float fact_kj = sinangle * mag_rkj;
    // avoid division by zero
    fact_ij = std::max(static_cast<float>(1e-12), fact_ij);
    fact_kj = std::max(static_cast<float>(1e-12), fact_kj);

    for (int i = 0; i < 3; ++i) {
      Float derv0 = deriv * (rij.get_component(i) * cosangle
                             - rkj.get_component(i)) / fact_ij;
      Float derv2 = deriv * (rkj.get_component(i) * cosangle
                             - rij.get_component(i)) / fact_kj;
      d0.add_to_coordinate_derivative(i, derv0, *accum);
      d1.add_to_coordinate_derivative(i, -derv0 - derv2, *accum);
      d2.add_to_coordinate_derivative(i, derv2, *accum);
    }
  } else {
    score = (*score_func_)(angle);
  }
  return score;
}


//! Show the current restraint.
/** \param[in] out Stream to send restraint description to.
 */
void AngleRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "angle restraint (active):" << std::endl;
  } else {
    out << "angle restraint (inactive):" << std::endl;
  }

  out << "  version: " << version() << "  ";
  out << "  last_modified_by: " << last_modified_by() << std::endl;
  out << "  particles: " << get_particle(0)->get_index();
  out << ", " << get_particle(1)->get_index();
  out << " and " << get_particle(2)->get_index();
  out << "  ";
  score_func_->show(out);
  out << std::endl;
}

}  // namespace IMP
