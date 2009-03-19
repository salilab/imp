/**
 *  \file Sphere3DPatch.h   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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
  Sphere3DPatch(){}
  //! Constructor
  /**
  /param[in] sph            the sphere to cut a patch from
  /param[in] crossing_plane the plane that crosses the sphere
  */
  Sphere3DPatch(const Sphere3D &sph, const Plane3D& crossing_plane);
  //! Return true if the point is inside the patch
  /** Note that the point must be on the sphere (this is not necessarily
      checked).
   */
  bool get_contains(const Vector3D &p) const;
  Plane3D  get_plane() const {return crossing_plane_;}
  Sphere3D get_sphere() const {return Sphere3D(sph_);}
  void show(std::ostream &out=std::cout) const {
    sph_.show();
    crossing_plane_.show();
  }
  //! Get a point which is on the boundary of the patch
  Vector3D get_boundary_point() const;
protected:
  Sphere3D sph_;
  Plane3D crossing_plane_;
};

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_SPHERE_3DPATCH_H */
