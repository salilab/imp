/**
 *  \file IMP/core/XYZR.h
 *  \brief Decorator for a sphere-like particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_XYZ_R_H
#define IMPCORE_XYZ_R_H

#include "XYZ.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/display/particle_geometry.h>
#include <IMP/display/geometry_macros.h>
#include <IMP/display/primitive_geometries.h>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle with x,y,z coordinates and a radius.
/** \ingroup decorators

    A simple example illustrating some of the functionality.
    \include XYZR_Decorator.py
 */
class IMPCOREEXPORT XYZR : public XYZ {
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::Sphere3D s) {
    XYZ::setup_particle(m, pi, s.get_center());
    do_setup_particle(m, pi, s.get_radius());
  }

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                double r) {
    m->add_attribute(get_radius_key(), pi, r, false);
  }

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi) {
    if (!XYZ::get_is_setup(m, pi)) {
      XYZ::setup_particle(m, pi);
    }
    m->add_attribute(get_radius_key(), pi, 0, false);
  }


 public:
  IMP_DECORATOR_METHODS(XYZR, XYZ);
  IMP_DECORATOR_SETUP_0(XYZR);
  /** Setup an XYZ particle as an XYZR particle. */
  IMP_DECORATOR_SETUP_1(XYZR, Float, radius);
  IMP_DECORATOR_SETUP_1(XYZR, algebra::Sphere3D, ball);

  /** Add the coordinates and radius from the sphere to the particle.
   */

  //! Check if the particle has the required attributes
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_radius_key(), pi);
  }

  double get_radius() const { return get_sphere().get_radius(); }
  void set_radius(double r) const {
    get_model()->get_sphere(get_particle_index())[3] = r;
  }

  //! Return a sphere object
  const algebra::Sphere3D &get_sphere() const {
    return get_model()->get_sphere(get_particle_index());
  }

  //! Set the attributes from a sphere
  void set_sphere(const algebra::Sphere3D &s) {
    get_model()->get_sphere(get_particle_index()) = s;
  }
  /** Get the key for the radius. */
  static FloatKey get_radius_key() { return IMP::internal::xyzr_keys[3]; }
  void add_to_radius_derivative(double v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_radius_key(), v, d);
  }
};

IMP_DECORATORS(XYZR, XYZRs, XYZs);

//! Compute the sphere distance between a and b
/** Compute the spere distance between a and b
    See XYZR
 */
inline double get_distance(XYZR a, XYZR b) {
  return IMP::algebra::get_distance(a.get_sphere(), b.get_sphere());
}

//! Set the coordinates and radius of the first to enclose the list
/** \param[in] v The vector of XYZ or XYZR particles to enclose
    \param[out] b The one whose values should be set
    \param[in] slack An amount to add to the radius.
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \note This function produces tighter bounds if the \ref cgal "CGAL"
    library is available.
    \ingroup CGAL
    See XYZR
 */
IMPCOREEXPORT void set_enclosing_sphere(XYZR b, const XYZs &v,
                                        double slack = 0);

//! Set the radius of the first to enclose the list
/** \param[in] v The vector of XYZ or XYZR particles to enclose
    \param[out] b The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    See XYZR
 */
IMPCOREEXPORT void set_enclosing_radius(XYZR b, const XYZs &v);

//! Get a sphere enclosing the set of XYZRs
/** \param[in] v The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    See XYZR
 */
IMPCOREEXPORT algebra::Sphere3D get_enclosing_sphere(const XYZs &v);

//! Create a set of particles with random coordinates
/** This function is mostly to be used to keep demo code brief.
    \param[in] m The model to add them to.
    \param[in] num The number of particles to create.
    \param[in] radius The radius to give them.
    \param[in] box_side The particles have coordinates from -box_side
    to box_side.
    See XYZR

    The particles coordinates are optimized.
 */
IMPCOREEXPORT XYZRs create_xyzr_particles(kernel::Model *m, unsigned int num,
                                          Float radius, Float box_side = 10);

/** \genericgeometry */
inline const algebra::Sphere3D get_sphere_d_geometry(XYZR d) {
  return d.get_sphere();
}

/** \genericgeometry */
inline void set_sphere_d_geometry(XYZR d, const algebra::Sphere3D &v) {
  d.set_sphere(v);
}

  /** \class XYZRGeometry
      \brief Display an IMP::core::XYZR particle as a ball.

      \class XYZRsGeometry
      \brief Display an IMP::SingletonContainer of IMP::core::XYZR particles
      as balls.
  */
IMP_PARTICLE_GEOMETRY(XYZR, core::XYZR, {
  display::SphereGeometry *g = new display::SphereGeometry(d.get_sphere());
  ret.push_back(g);
});

IMP_PARTICLE_GEOMETRY(XYZDerivative, core::XYZ, {
  algebra::Segment3D s(d.get_coordinates(),
                       d.get_coordinates() + d.get_derivatives());
  display::Geometry *g = new display::SegmentGeometry(s);
  ret.push_back(g);
});

  /** \class EdgePairGeometry
      \brief Display a segment connecting a pair of particles.

      \class EdgePairsGeometry
      \brief Display a segment for each pair in a
      IMP::kernel::PairContainer.
  */
IMP_PARTICLE_PAIR_GEOMETRY(EdgePair, core::XYZ, {
  ret.push_back(new display::SegmentGeometry(
      algebra::Segment3D(d0.get_coordinates(), d1.get_coordinates())));
});

IMPCORE_END_NAMESPACE

#ifndef SWIG
// swig doesn't like having the overloads in different namespaces
// it will do the conversion implicitly anyway
IMPKERNEL_BEGIN_NAMESPACE
/** \genericgeometry */
inline const algebra::Sphere3D get_sphere_d_geometry(kernel::Particle *p) {
  return core::XYZR(p).get_sphere();
}

/** \genericgeometry */
inline void set_sphere_d_geometry(kernel::Particle *p, const algebra::Sphere3D &v) {
  core::XYZR(p).set_sphere(v);
}

/** \genericgeometry */
inline const algebra::BoundingBoxD<3> get_bounding_box_d_geometry(kernel::Particle *p) {
  return get_bounding_box(core::XYZR(p).get_sphere());
}
IMPKERNEL_END_NAMESPACE
#endif

#endif /* IMPCORE_XYZ_R_H */
