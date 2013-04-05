/**
 *  \file IMP/display/primitive_geometries.h
 *  \brief Implement geometry for the basic shapes from IMP.algebra.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_PRIMITIVE_GEOMETRIES_H
#define IMPDISPLAY_PRIMITIVE_GEOMETRIES_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "geometry_macros.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Cylinder3D.h>
#include <IMP/algebra/Ellipsoid3D.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Triangle3D.h>
#include <IMP/algebra/standard_grids.h>
#ifdef IMP_DISPLAY_USE_IMP_CGAL
#include <IMP/cgal/internal/polyhedrons.h>
#endif

IMPDISPLAY_BEGIN_NAMESPACE


/** \brief Display a sphere.
*/
IMP_DISPLAY_GEOMETRY_DECL(SphereGeometry, SphereGeometries, algebra::Sphere3D);
/** \brief Display a cylinder.
*/
IMP_DISPLAY_GEOMETRY_DECL(CylinderGeometry, CylinderGeometries,
                          algebra::Cylinder3D);
/** \brief Display a ellipsoid.
*/
IMP_DISPLAY_GEOMETRY_DECL(EllipsoidGeometry, EllipsoidGeometries,
                          algebra::Ellipsoid3D);
/** \brief Display a point.
*/
IMP_DISPLAY_GEOMETRY_DECL(PointGeometry, PointGeometries,
                          algebra::Vector3D);
/** \brief Display a segment.
*/
IMP_DISPLAY_GEOMETRY_DECL(SegmentGeometry, SegmentGeometries,
                          algebra::Segment3D);
/** If CGAL is available, then any simple, planar polygon can be
    used. Otherwise, the polygons should be simple, planar and convex.
*/
IMP_DISPLAY_GEOMETRY_DECL(PolygonGeometry,PolygonGeometries,
                          algebra::Vector3Ds);
/** \brief Display a triangule.
*/
IMP_DISPLAY_GEOMETRY_DECL(TriangleGeometry,TriangleGeometries,
                          algebra::Triangle3D);
/** \brief Display a bounding box.
*/
IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(BoundingBoxGeometry,
                                       BoundingBoxGeometries,
                                       algebra::BoundingBoxD<3>);

/** \brief Display a reference frame.
*/
IMP_DISPLAY_GEOMETRY_DECOMPOSABLE_DECL(ReferenceFrameGeometry,
                                       ReferenceFrameGeometries,
                                       algebra::ReferenceFrame3D);

//! A text label for a ball in space
/** You can use the offset if the thing being labeled has a radius.
 */
class IMPDISPLAYEXPORT LabelGeometry: public Geometry {
  algebra::Sphere3D loc_;
  std::string text_;
 public:
  LabelGeometry(const algebra::Sphere3D &loc,
                std::string text);
  LabelGeometry(const algebra::Vector3D &loc,
                std::string text);
  std::string get_text() const {return text_;}
  const algebra::Sphere3D& get_location() const {return loc_;}
  IMP_GEOMETRY(LabelGeometry);
};



//! Display a surface mesh
/** Faces are delimited by -1s in the list of indices.
*/
class IMPDISPLAYEXPORT SurfaceMeshGeometry: public Geometry {
  const algebra::Vector3Ds vertices_;
  const Ints faces_;
protected:
  SurfaceMeshGeometry(const std::pair<algebra::Vector3Ds, Ints >&m,
                             std::string name="SurfaceMesh %1%");
 public:
  SurfaceMeshGeometry(const algebra::Vector3Ds& vertices,
                      const Ints &faces);
  const algebra::Vector3Ds& get_vertexes() const {return vertices_;}
  const Ints& get_faces() const {return faces_;}
  IMP_GEOMETRY(SurfaceMeshGeometry);
};


#ifdef IMP_DISPLAY_USE_IMP_CGAL
//! Display a plane as truncated to a bounding box
/** This requires CGAL.
 */
class IMPDISPLAYEXPORT PlaneGeometry: public Geometry {
  algebra::Plane3D plane_;
  algebra::BoundingBox3D bb_;
 public:
  PlaneGeometry(const algebra::Plane3D &loc,
                const algebra::BoundingBox3D& box);
  IMP_GEOMETRY(PlaneGeometry);
};



//! Display an isosurface of a density map
/** This requires CGAL.
 */
class IsosurfaceGeometry: public SurfaceMeshGeometry {
 public:
  template <int D,
            class Storage,
            class Value>
    IsosurfaceGeometry(const algebra::GridD<D, Storage, Value> &grid,
                       double iso):
    SurfaceMeshGeometry(cgal::internal::get_iso_surface(grid, iso),
                        "IsosurfaceGeometry %1%"){}
#ifdef SWIG
  IsosurfaceGeometry(const algebra::GridD<3,
                     algebra::DenseGridStorageD<3, double>, double >
                     &grid,
                     double iso);
  IsosurfaceGeometry(const algebra::GridD<3,
                     algebra::DenseGridStorageD<3, float>, float > &grid,
                     double iso);
#endif
};


//! Display an isosurface of a density map
/** This requires CGAL.
 */
class IMPDISPLAYEXPORT SkinSurfaceGeometry: public SurfaceMeshGeometry {
 public:
  SkinSurfaceGeometry(const algebra::Sphere3Ds &balls);
};

#endif


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PRIMITIVE_GEOMETRIES_H */
