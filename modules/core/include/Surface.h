/**
 *  \file IMP/core/Surface.h     \brief Simple surface decorator.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SURFACE_H
#define IMPCORE_SURFACE_H

#include <IMP/core/core_config.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/display/primitive_geometries.h>
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/algebra/Cylinder3D.h>
#include <IMP/Constraint.h>
#include <IMP/decorator_macros.h>
#include <IMP/Decorator.h>
#include <IMP/Particle.h>
#include <IMP/Object.h>
#include <IMP/Model.h>
#include <IMP/Pointer.h>


IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle that represents a surface, its coordinates, and orientation.
/** \ingroup decorators
    \see XYZ
    \see Direction
*/
class IMPCOREEXPORT Surface : public XYZ {
 public:
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::Vector3D &center = algebra::Vector3D(0, 0, 0),
                                const algebra::Vector3D &normal = algebra::Vector3D(0, 0, 1));

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf);

  IMP_DECORATOR_METHODS(Surface, XYZ);
  IMP_DECORATOR_SETUP_0(Surface);
  //! Set up the surface to align with the reference frame.
  IMP_DECORATOR_SETUP_1(Surface, algebra::ReferenceFrame3D, rf);
  IMP_DECORATOR_SETUP_1(Surface, algebra::Vector3D, center);
  IMP_DECORATOR_SETUP_2(Surface, algebra::Vector3D, center, algebra::Vector3D, normal);
  IMP_DECORATOR_GET_SET(normal_x, get_normal_key(0), Float, Float);
  IMP_DECORATOR_GET_SET(normal_y, get_normal_key(1), Float, Float);
  IMP_DECORATOR_GET_SET(normal_z, get_normal_key(2), Float, Float);

  //! Check if particle is setup as a surface.
  static bool get_is_setup(Model *m, ParticleIndex pi);

  static FloatKey get_normal_key(unsigned int i);

  //! Get the vector of derivatives added to the surface normal.
  Float get_normal_derivative(int i) const {
    return get_particle()->get_derivative(get_normal_key(i));
  }

  //! Get the vector of derivatives added to the surface normal.
  algebra::Vector3D get_normal_derivatives() const;

  //! Add v to the derivative of the ith coordinate of the normal.
  void add_to_normal_derivative(int i, Float v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_normal_key(i), v, d);
  }

  //! Add v to the derivatives of the x,y,z coordinates of the normal.
  void add_to_normal_derivatives(const algebra::Vector3D &v,
                                 DerivativeAccumulator &d);

  //! Get whether surface normal is optimized.
  bool get_normal_is_optimized() const;

  //! Set whether surface normal is optimized.
  void set_normal_is_optimized(bool tf) const;

  //! Get unit vector normal to surface.
  algebra::Vector3D get_normal() const;

  //! Set surface normal.
  void set_normal(const algebra::Vector3D &normal);

  //! Flip surface so normal is reversed.
  void reflect() { set_normal(-get_normal()); }

  //! Get height of point above surface.
  /** \note If point is below surface, value is negative. */
  double get_height(const algebra::Vector3D &v) const;

  //! Get depth of point below surface.
  /** \note If point is above surface, value is negative. */
  double get_depth(const algebra::Vector3D &v) const;

  //! Get distance from point to surface.
  double get_distance_to(const algebra::Vector3D &v) const;

  //! Get distance from point to center of surface.
  double get_distance_to_center(const algebra::Vector3D &v) const;

  //! Set coordinates and normal from a reference frame.
  void set_reference_frame(const algebra::ReferenceFrame3D &rf) {
    algebra::Vector3D c = rf.get_global_coordinates(algebra::Vector3D(0, 0, 0));
    algebra::Vector3D n = rf.get_global_coordinates(algebra::Vector3D(0, 0, 1)) - c;
    set_coordinates(c);
    set_normal(n);
  }

  //! Get surface orientation as a reference frame.
  algebra::ReferenceFrame3D get_reference_frame() const;

  //! Get a plane that lies along the surface.
  algebra::Plane3D get_plane() const {
    return algebra::Plane3D(get_coordinates(), get_normal());
  }
};

IMP_DECORATORS(Surface, Surfaces, XYZs);

//! Get height of sphere above surface.
/** \note If sphere crosses or is below surface, height is
    negative the distance from the surface to the furthest point
    of the sphere.
*/
inline double get_height(const Surface &s, const XYZR &d) {
  return s.get_height(d.get_coordinates()) - d.get_radius();
}

//! Get height of point above surface.
inline double get_height(const Surface &s, const XYZ &d) {
  return s.get_height(d.get_coordinates());
}

//! Get depth of sphere below surface.
/** \note If sphere crosses or is above surface, depth is
    negative the distance from the surface to the furthest point
    of the sphere.
*/
inline double get_depth(const Surface &s, const XYZR &d) {
  return s.get_depth(d.get_coordinates()) - d.get_radius();
}

//! Get depth of point below surface.
inline double get_depth(const Surface &s, const XYZ &d) {
  return s.get_depth(d.get_coordinates());
}

//! Get distance from sphere to surface.
inline double get_distance(const Surface &s, const XYZR &d) {
  return s.get_distance_to(d.get_coordinates()) - d.get_radius();
}

//! Get distance from point to surface.
inline double get_distance(const Surface &s, const XYZ &d) {
  return s.get_distance_to(d.get_coordinates());
}

/** \class SurfaceGeometry
    \brief Display a Surface particle as a cylindrical disk.

    \class SurfacesGeometry
    \brief Display an IMP::SingletonContainer of Surface particles
    as cylindrical disks.
*/
class IMPCOREEXPORT SurfaceGeometry : public display::Geometry {
  Surface s_;
  display::SegmentGeometry *n_;
  display::CylinderGeometry *c_;

  //! Get disk aligned with surface plane for geometry.
  algebra::Cylinder3D get_cylinder(double radius=10.0,
                                   double thick=0.1) const;

  //! Get line segment along surface normal for geometry.
  algebra::Segment3D get_segment(double length=5.0) const;

 public:
  SurfaceGeometry(std::string n="SurfaceGeometry");
  SurfaceGeometry(Surface s, const std::string n="SurfaceGeometry");
  SurfaceGeometry(Surface s, const display::Color &c, std::string n="SurfaceGeometry");
  virtual const Surface get_geometry() const { return s_; }
  void set_geometry(Surface s);
  virtual display::Geometries get_components() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SurfaceGeometry);
};
//! Create a geometry from a Surface
inline SurfaceGeometry* create_geometry(
    const Surface s, std::string name = std::string("SurfaceGeometry%1%")) {
  IMP_NEW(SurfaceGeometry, g, (s, name));
  return g.release();
}
IMP_OBJECTS(SurfaceGeometry, SurfaceGeometries)

//! Constrain a SurfaceGeometry to a Surface.
/** \note Use IMP.core.get_constrained_surface_geometry to create.
*/
class IMPCOREEXPORT SurfaceGeometryConstraint : public IMP::Constraint {
  ParticleIndex pi_;
  IMP::PointerMember<SurfaceGeometry> g_;

  public:
    SurfaceGeometryConstraint(Surface s, SurfaceGeometry* g)
      : IMP::Constraint(s.get_model(), "SurfaceGeometryConstraint%1%")
      , pi_(s.get_particle_index()), g_(g) {}

    Surface get_surface() const { return Surface(get_model(), pi_); }

    SurfaceGeometry* get_geometry() const { return g_; }

    virtual void do_update_attributes() IMP_OVERRIDE {};
    virtual void do_update_derivatives(DerivativeAccumulator *) IMP_OVERRIDE {};
    virtual void do_before_evaluate() IMP_OVERRIDE { g_->set_geometry(get_surface());}
    virtual void do_after_evaluate(DerivativeAccumulator *) IMP_OVERRIDE {
      g_->set_geometry(get_surface());
    };


    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
      ParticlesTemp ps = ParticlesTemp(1, get_surface().get_particle());
      return ps;
    }

    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
      ParticlesTemp ps = ParticlesTemp(1, get_surface().get_particle());
      return ps;
    }

   IMP_OBJECT_METHODS(SurfaceGeometryConstraint);
};

//! Get surface geometry constrained to the surface.
inline SurfaceGeometry* get_constrained_surface_geometry(
  const Surface s, std::string name="SurfaceGeometry%1%") {
  IMP_NEW(SurfaceGeometry, g, (s, name));
  IMP_NEW(SurfaceGeometryConstraint, c, (s, g));
  s.get_model()->add_score_state(c.release());
  return g.release();
}
IMP_OBJECTS(SurfaceGeometryConstraint, SurfaceGeometryConstraints);

//! Constrain the center of a Surface for visualization.
/** The Surface center can wander laterally. This constraint ensures
    the center is placed in the Surface such that it is along the
    normal from a point.
*/
class IMPCOREEXPORT LateralSurfaceConstraint : public IMP::Constraint {
  ParticleIndex spi_;
  ParticleIndex dpi_;

  public:
    LateralSurfaceConstraint(Surface s, XYZ d)
      : IMP::Constraint(s.get_model(), "LateralSurfaceConstraint%1%")
      , spi_(s.get_particle_index()), dpi_(d.get_particle_index()) {}

    virtual void do_update_attributes() IMP_OVERRIDE {}
    virtual void do_update_derivatives(DerivativeAccumulator *) IMP_OVERRIDE {}
    virtual void do_before_evaluate() IMP_OVERRIDE {
      algebra::Vector3D d = XYZ(get_model(), dpi_).get_coordinates();
      Surface s = Surface(get_model(), spi_);
      s.set_coordinates(s.get_plane().get_projected(d));
    }
    virtual void do_after_evaluate(DerivativeAccumulator *) IMP_OVERRIDE {}

    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
      ParticlesTemp ps;
      ps.push_back(get_model()->get_particle(dpi_));
      ps.push_back(get_model()->get_particle(spi_));
      return ps;
    }

    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
      ParticlesTemp ps = ParticlesTemp(1, get_model()->get_particle(spi_));
      return ps;
    }

   IMP_OBJECT_METHODS(LateralSurfaceConstraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SURFACE_H */
