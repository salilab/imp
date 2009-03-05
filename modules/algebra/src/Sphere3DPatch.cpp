/**
 *  \file Sphere3DPatch.cpp   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/algebra/Sphere3DPatch.h"
IMPALGEBRA_BEGIN_NAMESPACE
Sphere3DPatch::Sphere3DPatch(const Sphere3D &sph,
                            const Plane3D& crossing_plane) : sph_(sph) {
  crossing_plane_ = crossing_plane;
}
bool Sphere3DPatch::get_contains(const Vector3D &p) const {
  if (!sph_.get_contains(p)) {
    return false;
  }
  //check that the point is above the plane (the direction on the normal)
  return (crossing_plane_.is_above(p));
}
IMPALGEBRA_END_NAMESPACE
