/**
 *  \file Sphere3DPatch.h   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_SPHERE_3DPATCH_H
#define IMPALGEBRA_SPHERE_3DPATCH_H

#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/Plane3D.h>

IMPALGEBRA_BEGIN_NAMESPACE

/**
A patch of a sphere is defined as all points above the plane and in the sphere
 */
class IMPALGEBRAEXPORT Sphere3DPatch {
public:
  //! Constructor
  /**
  /param[in] sph            the sphere to cut a patch from
  /param[in] crossing_plane the plane that crosses the sphere
  */
  Sphere3DPatch(const Sphere3D &sph, const Plane3D& crossing_plane);
  //! Return true if the point is inside the patch
  bool get_contains(const Vector3D &p) const;
  const Plane3D &get_plane() const {return crossing_plane_;}
  const Sphere3D &get_sphere() const {return sph_;}
protected:
  Sphere3D sph_;
  Plane3D crossing_plane_;
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_SPHERE_3DPATCH_H */
