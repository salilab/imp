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

#include <limits>

IMPCORE_BEGIN_NAMESPACE

//! A a decorator for a particle with x,y,z coordinates and a radius.
/** \ingroup helper
    \ingroup decorators
 */
class IMPCOREEXPORT XYZRDecorator: public XYZDecorator
{
  FloatKey my_radius_;
public:
  /**
     \param[in] p The particle to wrap.
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  XYZRDecorator(Particle *p,
                FloatKey radius_key= FloatKey("radius")): XYZDecorator(p),
    my_radius_(radius_key) {
    IMP_assert(p->has_attribute(radius_key),
               "Missing attribute " << radius_key);
  }
  /** Cast a particle assuming that the key is radius_key.
     \param[in] p The particle to wrap.
     \param[in] radius_key The (optional) key name to use.
     The default is "radius".
   */
  static XYZRDecorator cast(Particle *p,
                            FloatKey radius_key= FloatKey("radius")) {
    XYZDecorator::cast(p);
    IMP_check(p->has_attribute(radius_key),
              "Missing radius attribute " << radius_key,
              InvalidStateException);
    return XYZRDecorator(p, radius_key);
  }
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

  //! Check if the particle has the required attributes
  static bool is_instance_of(Particle *p,
                             FloatKey radius_key= FloatKey("radius")) {
    return p->has_attribute(radius_key);
  }
  IMP_DECORATOR_GET_SET(radius, my_radius_, Float, Float);
  //! Get the radius key used in this decorator
  FloatKey get_radius_key() const {
    return my_radius_;
  }
  //! Get the default radius key.
  static FloatKey get_default_radius_key() {
    static FloatKey rk("radius");
    return rk;
  }

  //! show the values
  void show(std::ostream &out= std::cout, std::string prefix="") const;
};

IMP_OUTPUT_OPERATOR(XYZRDecorator);

//! Compute the distance between a pair of particles
/** \ingroup helper
    \ingroup uncommitted
 */
IMPCOREEXPORT Float distance(XYZRDecorator a, XYZRDecorator b);

//! Set the coordinates and radius of the last to enclose the list
/** \ingroup helper
    \param[in] v The vector of XYZR objects to enclose
    \param[out] b The one whose values should be set

    \note this takes a Particles object rather than a vector of
    something else since you can't easily cast vectors of
    different things to one another. Well, you can cast vectors
    of decorators in C++, but you can't in python without adding
    explicit support.
    \ingroup uncommitted
 */
IMPCOREEXPORT void set_enclosing_sphere(const Particles &v,
                                       XYZRDecorator b);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_XYZR_DECORATOR_H */
