/**
 *  \file XYZRDecorator.h
 *  \brief Decorator for a sphere-like particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_XYZR_DECORATOR_H
#define IMPCORE_XYZR_DECORATOR_H

#include "XYZDecorator.h"
#include <IMP/algebra/Sphere3D.h>

#include <limits>

IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle with x,y,z coordinates and a radius.
/** \ingroup decorators
    A simple example illustrating some of the functionality.
    \verbinclude xyzrdecorator.py
 */
class IMPCOREEXPORT XYZRDecorator: public XYZDecorator
{
public:
  IMP_DECORATOR_TRAITS(XYZRDecorator, XYZDecorator, FloatKey,
                       radius_key, get_default_radius_key());

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static XYZRDecorator create(Particle *p,
                              FloatKey radius_key= FloatKey("radius")) {
    if (!XYZDecorator::is_instance_of(p)) {
      XYZDecorator::create(p);
    }
    p->add_attribute(radius_key, 0, false);
    return XYZRDecorator(p, radius_key);
  }


 /** Create a decorator using radius_key to store the FloatKey.
     The particle should already be an XYZDecorator particle.
     \param[in] p The particle to wrap.
     \param[in] radius The radius to set initially
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static XYZRDecorator create(Particle *p,
                              Float radius,
                              FloatKey radius_key= FloatKey("radius")) {
    p->add_attribute(radius_key, radius, false);
    return XYZRDecorator(p, radius_key);
  }

  /** Create a decorator using radius_key to store the FloatKey.
     \param[in] p The particle to wrap.
     \param[in] s The sphere to use to set the position and radius
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static XYZRDecorator create(Particle *p,
                              const algebra::Sphere3D &s,
                              FloatKey radius_key= FloatKey("radius")) {
    XYZDecorator::create(p, s.get_center());
    p->add_attribute(radius_key, s.get_radius(), false);
    return XYZRDecorator(p, radius_key);
  }

  //! Check if the particle has the required attributes
  static bool is_instance_of(Particle *p,
                             FloatKey radius_key= FloatKey("radius")) {
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
    static FloatKey rk("radius");
    return rk;
  }
};

IMP_OUTPUT_OPERATOR(XYZRDecorator);

//! Compute the distance between a pair of particles
/** \relates XYZRDecorator
 */
IMPCOREEXPORT Float distance(XYZRDecorator a, XYZRDecorator b);

//! Set the coordinates and radius of the last to enclose the list
/** \param[in] v The vector of XYZ or XYZR particles to enclose
    \param[out] b The one whose values should be set
    Any particle which does not have the attribute b.get_radius()
    is assumed to have a radius of 0.

    \note This takes a Particles object rather than a vector of
    something else since you can't easily cast vectors of
    different things to one another.

    \note This function produces tighter bounds if the CGAL library
    is linked.

    \relates XYZRDecorator
 */
IMPCOREEXPORT void set_enclosing_sphere(XYZRDecorator b,
                                        const Particles &v);

//! Create a set of particles which random coordinates
/** This function is mostly to be used to keep demo code brief.
    \param[in] m The model to add them to.
    \param[in] num The number of particles to create.
    \param[in] radius The radius to give them.
    \param[in] box_side The particles have coordinates from -box_side
    to box_side.
    \relates XYZRDecorator
 */
IMPCOREEXPORT Particles create_xyzr_particles(Model *m,
                                              unsigned int num,
                                              Float radius,
                                              Float box_side=10);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_XYZR_DECORATOR_H */
