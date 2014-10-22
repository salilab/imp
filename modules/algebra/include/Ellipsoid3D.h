/**
 *  \file IMP/algebra/Ellipsoid3D.h   \brief Simple 3D ellipsoid class.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ELLIPSOID_3D_H
#define IMPALGEBRA_ELLIPSOID_3D_H

#include <IMP/algebra/algebra_config.h>
#include "constants.h"
#include "BoundingBoxD.h"
#include "Transformation3D.h"
#include "ReferenceFrame3D.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Represent an ellipsoid in 3D.
/** \geometry */
class IMPALGEBRAEXPORT Ellipsoid3D : public GeometricPrimitiveD<3> {
 public:
  Ellipsoid3D() {}
  Ellipsoid3D(const ReferenceFrame3D &rf, const algebra::Vector3D &radii)
      : rf_(rf), radii_(radii) {}
  Ellipsoid3D(const Vector3D &center, double radius_x, double radius_y,
              double radius_z, const Rotation3D &rot);
  const Vector3D &get_radii() const { return radii_; }
  const ReferenceFrame3D &get_reference_frame() const { return rf_; }
  /** \deprecated_at{2.2} Use get_radii()*/
  IMPALGEBRA_DEPRECATED_FUNCTION_DECL(2.2)
  double get_radius(unsigned int i) const { return radii_[i]; }
  /** \deprecated_at{2.2} Use get_reference_frame()*/
  IMPALGEBRA_DEPRECATED_FUNCTION_DECL(2.2)
  const Vector3D &get_center() const {
    return rf_.get_transformation_to().get_translation();
  }
  /** \deprecated_at{2.2} Use get_reference_frame()*/
  IMPALGEBRA_DEPRECATED_FUNCTION_DECL(2.2)
  const Rotation3D &get_rotation() const {
    return rf_.get_transformation_to().get_rotation();
  }
  /** \deprecated_at{2.2} Use get_reference_frame()*/
  IMPALGEBRA_DEPRECATED_FUNCTION_DECL(2.2)
  const Transformation3D get_transformation() const {
    return rf_.get_transformation_to();
  }
  IMP_SHOWABLE_INLINE(Ellipsoid3D, out << rf_ << ": " << radii_);

 private:
  ReferenceFrame3D rf_;
  Vector3D radii_;
};

IMP_VOLUME_GEOMETRY_METHODS(Ellipsoid3D, ellipsoid_3d, IMP_UNUSED(g);
                            IMP_NOT_IMPLEMENTED,
                            return 4.0 / 3.0 * PI * g.get_radii()[0] *
                                   g.get_radii()[1] * g.get_radii()[2],
{
  Vector3D v = g.get_radii();
  BoundingBoxD<3> b(-v, v);
  return get_transformed(b, g.get_reference_frame().get_transformation_to());
});

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_ELLIPSOID_3D_H */
