/**
 *  \file IMP/algebra/Ellipsoid3D.h   \brief Simple 3D ellipsoid class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_ELLIPSOID_3D_H
#define IMPALGEBRA_ELLIPSOID_3D_H

#include "Cylinder3D.h"
#include <cmath>
#include "constants.h"
#include "BoundingBoxD.h"
#include "Transformation3D.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent an ellispoid in 3D.
    \geometry
  */
class IMPALGEBRAEXPORT Ellipsoid3D: public GeometricPrimitiveD<3> {
public:
  Ellipsoid3D(){
#if IMP_HAS_CHECKS >= IMP_USAGE
    radii_[0]= std::numeric_limits<double>::quiet_NaN();
#endif
  }
  Ellipsoid3D(const Vector3D& center,double radius_x,
              double radius_y, double radius_z,
              const Rotation3D &rot);
  double get_radius(unsigned int i) const {
    IMP_USAGE_CHECK(!base::isnan(radii_[0]),
              "Attempt to use uninitialized ellipsoid.");
    return radii_[i];
  }
  const Vector3D &get_center() const {return center_;}
  const Rotation3D &get_rotation() const {
    return rot_;
  }
  const Transformation3D get_transformation() const  {
    return Transformation3D(rot_, center_);
  }
  IMP_SHOWABLE_INLINE(Ellipsoid3D, {
      out << "(" << spaces_io(center_) << ": " << get_radius(0)
          << ", " << get_radius(1) << ", " << get_radius(2)
          << ")";
    });
private:
  Vector3D center_;
  double radii_[3];
  Rotation3D rot_;
};

IMP_VOLUME_GEOMETRY_METHODS(Ellipsoid3D, ellipsoid_3d,
                            if (0) std::cout << g; IMP_NOT_IMPLEMENTED,
                            return 4.0/3.0 * PI *g.get_radius(0)
                            *g.get_radius(1)*g.get_radius(2),
                            {
                              Vector3D v(g.get_radius(0),
                                         g.get_radius(1),
                                         g.get_radius(2));
                              BoundingBoxD<3> b(-v, v);
                              return get_transformed(b, g.get_transformation());
                            });

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_ELLIPSOID_3D_H */
