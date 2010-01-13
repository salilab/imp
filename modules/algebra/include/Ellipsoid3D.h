/**
 *  \file Ellipsoid3D.h   \brief Simple 3D ellipsoid class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_ELLIPSOID_3D_H
#define IMPALGEBRA_ELLIPSOID_3D_H

#include "Cylinder3D.h"
#include <cmath>
#include <IMP/constants.h>
#include "internal/cgal_predicates.h"
#include "BoundingBoxD.h"
#include "Rotation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent an ellispoid in 3D.
    \ingroup uninitialized_default
  */
class IMPALGEBRAEXPORT Ellipsoid3D {
public:
  Ellipsoid3D(){
#if IMP_BUILD < IMP_FAST
    radii_[0]= std::numeric_limits<double>::quiet_NaN();
#endif
  }
  Ellipsoid3D(const Vector3D& center,double radius_x,
              double radius_y, double radius_z,
              const Rotation3D &rot);
  double get_radius(unsigned int i) const {
    IMP_USAGE_CHECK(!is_nan(radii_[0]),
              "Attempt to use uninitialized ellipsoid.",
              InvalidStateException);
    return radii_[i];
  }
  const Vector3D &get_center() const {return center_;}
  const Rotation3D &get_rotation() const {
    return rot_;
  }
  IMP_SHOWABLE_INLINE({
      out << "(" << spaces_io(center_) << ": " << get_radius(0)
          << ", " << get_radius(1) << ", " << get_radius(2)
          << ")";
    })
private:
  Vector3D center_;
  double radii_[3];
  Rotation3D rot_;
};

IMP_OUTPUT_OPERATOR(Ellipsoid3D);

typedef std::vector<Ellipsoid3D> Ellipsoid3Ds;

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_ELLIPSOID_3D_H */
