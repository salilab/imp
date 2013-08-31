/**
 *  \file IMP/misc/CustomXYZR.h
 *  \brief Decorator for a sphere-like particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMISC_CUSTOM_XYZ_R_H
#define IMPMISC_CUSTOM_XYZ_R_H

#include <IMP/misc/misc_config.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/algebra/Sphere3D.h>

#include <limits>

IMPMISC_BEGIN_NAMESPACE

//! A decorator for a particle with x,y,z coordinates and a radius.
/** This is mostly just a sample decorator.
 */
class IMPMISCEXPORT CustomXYZR : public core::XYZ {
   static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                 FloatKey radius_key) {
     if (!XYZ::get_is_setup(m, pi)) {
       XYZ::setup_particle(m, pi);
     }
     m->add_attribute(radius_key, pi, 0, false);
  }
   static void do_setup_particle(Model *m, kernel::ParticleIndex pi, Float radius,
                                   FloatKey radius_key) {
    if (!XYZ::get_is_setup(m, pi)) {
      XYZ::setup_particle(m, pi);
    }
    m->add_attribute(radius_key, pi, radius, false);
  }
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                   const algebra::Sphere3D &s,
                                   FloatKey radius_key) {
    XYZ::setup_particle(m, pi, s.get_center());
    m->add_attribute(radius_key, pi, s.get_radius(), false);
  }
 public:
  IMP_DECORATOR_WITH_TRAITS_METHODS(CustomXYZR, core::XYZ, FloatKey, radius_key,
                                    IMP::core::XYZR::get_radius_key());
  IMP_DECORATOR_TRAITS_SETUP_0(CustomXYZR);
  IMP_DECORATOR_TRAITS_SETUP_1(CustomXYZR, double, radius);
  IMP_DECORATOR_TRAITS_SETUP_1(CustomXYZR, algebra::Sphere3D, ball);

  //! Check if the particle has the required attributes
  static bool get_is_setup(Model *m, kernel::ParticleIndex pi,
                           FloatKey radius_key =
                           get_default_radius_key()) {
    return m->get_has_attribute(radius_key, pi);
  }
  IMP_DECORATOR_GET_SET(radius, get_decorator_traits(), Float, Float);

  //! Return a sphere object
  algebra::Sphere3D get_sphere() const {
    return algebra::Sphere3D(get_coordinates(), get_radius());
  }

  //! Set the attributes from a sphere
  void set_sphere(const algebra::Sphere3D &s) {
    set_coordinates(s.get_center());
    set_radius(s.get_radius());
  }
  //! Get the default radius key.
  static FloatKey get_default_radius_key() {
    return IMP::internal::xyzr_keys[3];
  }
  void add_to_radius_derivative(double v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_decorator_traits(), v, d);
  }
  FloatKey get_traits() { return get_decorator_traits(); }
};

IMP_DECORATORS(CustomXYZR, CustomXYZRs, core::XYZs);

//! Compute the distance between a pair of particles
/** See XYZR
 */
inline double get_distance(CustomXYZR a, CustomXYZR b) {
  return IMP::algebra::get_distance(a.get_sphere(), b.get_sphere());
}

/** \genericgeometry */
inline const algebra::Sphere3D get_sphere_d_geometry(CustomXYZR d) {
  return d.get_sphere();
}

/** \genericgeometry */
inline void set_sphere_d_geometry(CustomXYZR d, const algebra::Sphere3D &v) {
  d.set_sphere(v);
}

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_CUSTOM_XYZ_R_H */
