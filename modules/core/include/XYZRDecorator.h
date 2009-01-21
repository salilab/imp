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
#include <IMP/algebra/Vector3D.h>

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
     \param[in] p The particle to wrap.
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static XYZRDecorator create(Particle *p,
                              const algebra::Vector3D &coords,
                              Float radius,
                              FloatKey radius_key= FloatKey("radius")) {
    if (!XYZDecorator::is_instance_of(p)) {
      XYZDecorator::create(p, coords);
    }
    p->add_attribute(radius_key, radius, false);
    return XYZRDecorator(p, radius_key);
  }

  //! Check if the particle has the required attributes
  static bool is_instance_of(Particle *p,
                             FloatKey radius_key= FloatKey("radius")) {
    return p->has_attribute(radius_key);
  }
  IMP_DECORATOR_GET_SET(radius, get_radius_key(), Float, Float);

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
/** \param[in] v The vector of XYZR objects to enclose
    \param[out] b The one whose values should be set

    \note This takes a Particles object rather than a vector of
    something else since you can't easily cast vectors of
    different things to one another. Well, you can cast vectors
    of decorators in C++, but you can't in python without adding
    explicit support.

    \note This function produces tighter bounds if the CGAL library
    is linked.

    \relates XYZRDecorator
 */
IMPCOREEXPORT void set_enclosing_sphere(const Particles &v,
                                       XYZRDecorator b);

//! Create a set of particles which random coordinates
/** This function is mostly to be used to keep demo code brief.
    \param[in] m The model to add them to.
    \param[in] num The number of particles to create.
    \param[in] radius The radius to give them.
    \param[in] box_side The particles have coordinates from -box_side
    to box_side.
 */
IMPCOREEXPORT Particles create_xyzr_particles(Model *m,
                                              unsigned int num,
                                              Float radius,
                                              Float box_side=10);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_XYZR_DECORATOR_H */
