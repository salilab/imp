/**
 *  \file  Cylinder3D.cpp
 *  \brief stores a cylinder
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Cylinder3D.h>

IMPALGEBRA_BEGIN_NAMESPACE
Cylinder3D::Cylinder3D(const Segment3D &s,
                       double radius)
  :s_(s),radius_(radius){
}

IMPALGEBRA_END_NAMESPACE
