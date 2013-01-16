/**
 *  \file IMP/algebra/GeometricPrimitiveD.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GEOMETRIC_PRIMITIVE_D_H
#define IMPALGEBRA_GEOMETRIC_PRIMITIVE_D_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/Vector.h>
#include <IMP/base/utility_macros.h>

IMPALGEBRA_BEGIN_NAMESPACE
/** Geometric primitives in \imp behave a bit differently than most values.

    Most classes in \imp are initialized to a defined state (sometimes a null
    state) by their default constructor. For efficiency reasons, certain ones,
    mostly low-level geometric types, are not. They act like built in types
    in C++ in that they only have a defined state if set (or initialized).

    In addition, they are not comparable, or hashable.

    See the \ref geometricprimitives :Geometric Primitives" entry for more
    information.
*/
template <int D>
class GeometricPrimitiveD {
protected:
  GeometricPrimitiveD(){}
};

#ifndef IMP_DOXYGEN
  typedef GeometricPrimitiveD<1> GeometricPrimitive1D;
  typedef GeometricPrimitiveD<2> GeometricPrimitive2D;
  typedef GeometricPrimitiveD<3> GeometricPrimitive3D;
  typedef GeometricPrimitiveD<4> GeometricPrimitive4D;
  typedef GeometricPrimitiveD<5> GeometricPrimitive5D;
  typedef GeometricPrimitiveD<6> GeometricPrimitive6D;
  typedef GeometricPrimitiveD<-1> GeometricPrimitiveKD;
#endif

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GEOMETRIC_PRIMITIVE_D_H */
