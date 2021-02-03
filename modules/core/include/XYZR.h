/**
 *  \file IMP/core/XYZR.h
 *  \brief Decorator for a sphere-like particle.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::Sphere3D s) {
    XYZ::setup_particle(m, pi, s.get_center());
    do_setup_particle(m, pi, s.get_radius());
  }

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                double r) {
    m->add_attribute(get_radius_key(), pi, r, false);
  }

  static void do_setup_particle(Model *m, ParticleIndex pi) {
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
  static bool get_is_setup(Model *m, ParticleIndex pi) {
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
  //! Get the key for the radius.
  static FloatKey get_radius_key() { return IMP::internal::xyzr_keys[3]; }

  //! add v to the derivative of the radius attribute
  void add_to_radius_derivative(double v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_radius_key(), v, d);
  }
};

IMP_DECORATORS(XYZR, XYZRs, XYZs);

//! Compute the sphere distance between a and b
/** \see XYZR
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
    \see XYZR
 */
IMPCOREEXPORT void set_enclosing_sphere(XYZR b, const XYZs &v,
                                        double slack = 0);

//! Set the radius of the first to enclose the list
/** \param[in] v The vector of XYZ or XYZR particles to enclose
    \param[out] b The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \see XYZR
 */
IMPCOREEXPORT void set_enclosing_radius(XYZR b, const XYZs &v);

//! Get a sphere enclosing the set of XYZRs
/** \param[in] v The one whose radius should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \see XYZR
 */
IMPCOREEXPORT algebra::Sphere3D get_enclosing_sphere(const XYZs &v);

//! Create a set of particles with random coordinates
/** This function is mostly to be used to keep demo code brief.
    \param[in] m The model to add them to.
    \param[in] num The number of particles to create.
    \param[in] radius The radius to give them.
    \param[in] box_side The particles have coordinates from -box_side
    to box_side.
    \see XYZR

    The particles coordinates are optimized.
 */
IMPCOREEXPORT XYZRs create_xyzr_particles(Model *m, unsigned int num,
                                          Float radius, Float box_side = 10);

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
    IMP::PairContainer.
*/
IMP_PARTICLE_PAIR_GEOMETRY(EdgePair, core::XYZ, {
  ret.push_back(new display::SegmentGeometry(
      algebra::Segment3D(d0.get_coordinates(), d1.get_coordinates())));
});

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_XYZ_R_H */
