/**
 *  \file Transformation3D.cpp
 *  \brief Simple 3D transformation class.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Transformation3D.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/Sphere3D.h"
#include "IMP/algebra/vector_generators.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation3D::~Transformation3D() {}

void Transformation3D::get_transformed_adjoint(
    const Vector3D &v, const Vector3D &Dw,
    Vector3D *Dv, Transformation3DAdjoint *DT) const {
  if (!DT) return;
  rot_.get_rotated_adjoint(v, Dw, Dv, &(DT->first));
  std::copy(Dw.begin(), Dw.end(), DT->second.begin());
}

TransformedVector3DAdjoint
Transformation3D::get_transformed_adjoint(
    const Vector3D &v, const Vector3D &Dw) const {
  Vector3D Dv;
  Transformation3DAdjoint DT;
  get_transformed_adjoint(v, Dw, &Dv, &DT);
  return TransformedVector3DAdjoint(Dv, DT);
}

void compose_adjoint(const Transformation3D &TA, const Transformation3D &TB,
                     const Transformation3DAdjoint &DTC,
                     Transformation3DAdjoint *DTA,
                     Transformation3DAdjoint *DTB) {
  bool has_DTB = DTB;
  TA.get_transformed_adjoint(TB.get_translation(), DTC.second,
                             has_DTB ? &(DTB->second) : nullptr, DTA);
  if (DTA) {
    Rotation3DAdjoint DA;
    compose_adjoint(TA.get_rotation(), TB.get_rotation(), DTC.first, &DA,
                    has_DTB ? &(DTB->first) : nullptr);
    DTA->first += DA;
  } else {
    compose_adjoint(TA.get_rotation(), TB.get_rotation(), DTC.first, nullptr,
                    has_DTB ? &(DTB->first) : nullptr);
  }
}

ComposeTransformation3DAdjoint
compose_adjoint(const Transformation3D &TA, const Transformation3D &TB,
                const Transformation3DAdjoint &DTC) {
  Transformation3DAdjoint DTA, DTB;
  compose_adjoint(TA, TB, DTC, &DTA, &DTB);
  return ComposeTransformation3DAdjoint(DTA, DTB);
}

Transformation3D Transformation3D::get_inverse() const {
  Rotation3D inv_rot = rot_.get_inverse();
  return Transformation3D(inv_rot, -(inv_rot.get_rotated(trans_)));
}

Transformation3D get_transformation_3d(const Transformation2D &t2d) {
  Rotation3D R =
      get_rotation_from_fixed_zyz(t2d.get_rotation().get_angle(), 0.0, 0.0);
  Vector3D t(t2d.get_translation()[0], t2d.get_translation()[1], 0);
  return Transformation3D(R, t);
}
Transformation3D get_random_local_transformation(Vector3D origin,
                                                 double max_translation,
                                                 double max_angle_in_rad) {
  algebra::Vector3D translation = algebra::get_random_vector_in(
      algebra::Sphere3D(algebra::Vector3D(0, 0, 0), max_translation));
  algebra::Vector3D axis =
      algebra::get_random_vector_on(algebra::Sphere3D(origin, 1.));
  ::boost::uniform_real<> rand(-max_angle_in_rad, max_angle_in_rad);
  Float angle = rand(random_number_generator);
  algebra::Rotation3D r = algebra::get_rotation_about_axis(axis, angle);
  return algebra::Transformation3D(r, translation);
}

IMPALGEBRA_END_NAMESPACE
