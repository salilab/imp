/**
 *  \file utility.cpp
 *  \brief Miscellaneous utility functions.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/internal/utility.h>
#include <IMP/compatibility/math.h>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

namespace {
  bool rotation_is_valid(const TAU::Rotation3 &rot) {
    float sqr_mag=0.;
    for(int i=0;i<4;i++){
      sqr_mag+=rot[i]*rot[i];
      if (compatibility::isnan(rot[i])) {
        return false;
      }
    }
    return std::abs(sqr_mag-1.)<0.001;
  }
}

algebra::Transformation3D tau2imp(const TAU::RigidTrans3 &t) {
  TAU::Rotation3 rot = t.rotation_q();
  if (!rotation_is_valid(rot)) {
    return algebra::get_identity_transformation_3d();
  }
  return algebra::Transformation3D(
           algebra::Rotation3D(rot[0],rot[1],rot[2],rot[3]),
           algebra::Vector3D(t.translation()[0],t.translation()[1],
                             t.translation()[2]));
}

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE
