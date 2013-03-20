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
    \pythonexample{XYZR_Decorator}
 */
class IMPCOREEXPORT XYZR:
  public XYZ
{
public:
  IMP_DECORATOR(XYZR, XYZ);

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
   */
  static XYZR setup_particle(Particle *p) {
    if (!XYZ::particle_is_instance(p)) {
      XYZ::setup_particle(p);
    }
    p->add_attribute(get_radius_key(), 0, false);
    return XYZR(p);
  }


 /** Create a decorator using radius_key to store the FloatKey.
     The particle should already be an XYZ particle.
     \param[in] p The particle to wrap.
     \param[in] radius The radius to set initially
   */
  static XYZR setup_particle(Particle *p,
                     Float radius) {
    p->add_attribute(get_radius_key(), radius, false);
    return XYZR(p);
  }

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
     \param[in] s The sphere to use to set the position and radius
   */
  static XYZR setup_particle(Particle *p,
                             // See XYZ::setup_particle before you change this
                     const algebra::Sphere3D s) {
    XYZ::setup_particle(p, s.get_center());
    p->add_attribute(get_radius_key(), s.get_radius(), false);
    return XYZR(p);
  }

  /** Add the coordinates and radius from the sphere to the particle.
   */
  static XYZR setup_particle(Model *m, ParticleIndex pi,
                             // See XYZ::setup_particle before you change this
                             const algebra::Sphere3D s) {
    XYZ::setup_particle(m, pi, s.get_center());
    m->add_attribute(get_radius_key(), pi, s.get_radius(), false);
    return XYZR(m, pi);
  }

  //! Check if the particle has the required attributes
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_radius_key());
  }
  double get_radius() const {
    return get_sphere().get_radius();
  }
  void set_radius(double r) const {
     get_model()->get_sphere(get_particle_index())[3]=r;
  }


  //! Return a sphere object
  const algebra::Sphere3D& get_sphere() const {
    return get_model()->get_sphere(get_particle_index());
  }

  //! Set the attributes from a sphere
  void set_sphere(const algebra::Sphere3D &s) {
    get_model()->get_sphere(get_particle_index())=s;
  }
  //! Get the default radius key.
  static FloatKey get_radius_key() {
    return IMP::internal::xyzr_keys[3];
  }
  void add_to_radius_derivative(double v,
                                DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_radius_key(), v, d);
  }
};

IMP_DECORATORS(XYZR,XYZRs, XYZs);

//! Compute the distance between a pair of particles
/** \relatesalso XYZR
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
    \relatesalso XYZR
 */
IMPCOREEXPORT void set_enclosing_sphere(XYZR b,
                                        const XYZs &v,
                                        double slack=0);

//! Set the radius of the first to enclose the list
/** \param[in] v The vector of XYZ or XYZR particles to enclose
    \param[out] b The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \relatesalso XYZR
 */
IMPCOREEXPORT void set_enclosing_radius(XYZR b,
                                        const XYZs &v);

//! Get a sphere enclosing the set of XYZRs
/** \param[in] v The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \relatesalso XYZR
 */
IMPCOREEXPORT algebra::Sphere3D get_enclosing_sphere(const XYZs& v);

//! Create a set of particles with random coordinates
/** This function is mostly to be used to keep demo code brief.
    \param[in] m The model to add them to.
    \param[in] num The number of particles to create.
    \param[in] radius The radius to give them.
    \param[in] box_side The particles have coordinates from -box_side
    to box_side.
    \relatesalso XYZR

    The particles coordinates are optimized.
 */
IMPCOREEXPORT XYZRs create_xyzr_particles(Model *m,
                                          unsigned int num,
                                          Float radius,
                                          Float box_side=10);

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
IMP_PARTICLE_GEOMETRY(XYZR, core::XYZR,
 {
   display::SphereGeometry *g= new display::SphereGeometry(d.get_sphere());
   ret.push_back(g);
  });




IMP_PARTICLE_GEOMETRY(XYZDerivative, core::XYZ, {
    algebra::Segment3D s(d.get_coordinates(),
                         d.get_coordinates()+d.get_derivatives());
    display::Geometry *g= new display::SegmentGeometry(s);
    ret.push_back(g);
  });


/** \class EdgePairGeometry
    \brief Display an IMP::atom::Bond particle as a segment.

    \class EdgePairsGeometry
    \brief Display an IMP::SingletonContainer of IMP::atom::Bond particles
    as segments.
*/
IMP_PARTICLE_PAIR_GEOMETRY(EdgePair, core::XYZ, {
    ret.push_back(
        new display::SegmentGeometry(algebra::Segment3D(d0.get_coordinates(),
                                           d1.get_coordinates())));
  });

IMPCORE_END_NAMESPACE

#ifndef SWIG
// swig doesn't like having the overloads in different namespaces
// it will do the conversion implicitly anyway
IMPKERNEL_BEGIN_NAMESPACE
/** \genericgeometry */
inline const algebra::Sphere3D get_sphere_d_geometry(Particle *p) {
  return core::XYZR(p).get_sphere();
}

/** \genericgeometry */
inline void set_sphere_d_geometry(Particle *p, const algebra::Sphere3D &v) {
  core::XYZR(p).set_sphere(v);
}

/** \genericgeometry */
inline const algebra::BoundingBoxD<3>
get_bounding_box_d_geometry(Particle *p) {
  return get_bounding_box(core::XYZR(p).get_sphere());
}
IMPKERNEL_END_NAMESPACE
#endif

#endif  /* IMPCORE_XYZ_R_H */
