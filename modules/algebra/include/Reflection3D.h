/**
 *  \file IMP/algebra/Reflection3D.h   \brief Reflect about a plane in 3D.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_REFLECTION_3D_H
#define IMPALGEBRA_REFLECTION_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Plane3D.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Reflect about a plane in 3D
/**
*/
class Reflection3D: public GeometricPrimitiveD<3>
{
  Plane3D pl_;
public:
  Reflection3D(){}
  Reflection3D(Plane3D pl): pl_(pl){}

  Vector3D get_reflected(const Vector3D &v) const {
    Vector3D p= pl_.get_projection(v);
    return v+2*(p-v);
  }
  IMP_SHOWABLE_INLINE(Transformation3D, {
      out << pl_;
    }
    );
};

IMP_VALUES(Reflection3D, Reflection3Ds);

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_REFLECTION_3D_H */
