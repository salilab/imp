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
class IMPMISCEXPORT CustomXYZR:
  public core::XYZ
{
public:
  IMP_DECORATOR_WITH_TRAITS(CustomXYZR, core::XYZ, FloatKey,
                       radius_key, get_default_radius_key());

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static CustomXYZR setup_particle(Particle *p,
                     FloatKey radius_key= get_default_radius_key()) {
    if (!XYZ::particle_is_instance(p)) {
      XYZ::setup_particle(p);
    }
    p->add_attribute(radius_key, 0, false);
    return CustomXYZR(p, radius_key);
  }


 /** Create a decorator using radius_key to store the FloatKey.
     The particle should already be an XYZ particle.
     \param[in] p The particle to wrap.
     \param[in] radius The radius to set initially
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static CustomXYZR setup_particle(Particle *p,
                     Float radius,
                     FloatKey radius_key= get_default_radius_key()) {
    p->add_attribute(radius_key, radius, false);
    return CustomXYZR(p, radius_key);
  }

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
     \param[in] s The sphere to use to set the position and radius
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static CustomXYZR setup_particle(Particle *p,
                     const algebra::Sphere3D &s,
                     FloatKey radius_key= get_default_radius_key()) {
    XYZ::setup_particle(p, s.get_center());
    p->add_attribute(radius_key, s.get_radius(), false);
    return CustomXYZR(p, radius_key);
  }

  //! Check if the particle has the required attributes
  static bool particle_is_instance(Particle *p,
                             FloatKey radius_key= get_default_radius_key()) {
    return p->has_attribute(radius_key);
  }
  IMP_DECORATOR_GET_SET(radius, get_radius_key(), Float, Float);


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
  void add_to_radius_derivative(double v,
                                DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_radius_key(), v, d);
  }
  FloatKey get_traits() {
    return get_radius_key();
  }
};

IMP_DECORATORS(CustomXYZR,CustomXYZRs, core::XYZs);

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

#endif  /* IMPMISC_CUSTOM_XYZ_R_H */
