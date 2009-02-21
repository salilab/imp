/**
 *  \file XYZDecorator.h     \brief Simple xyz decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_XYZ_DECORATOR_H
#define IMPCORE_XYZ_DECORATOR_H

#include "config.h"
#include "internal/utility.h"

#include <IMP/Decorator.h>
#include <IMP/algebra/Vector3D.h>

#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

//! A a decorator for a particle with x,y,z coordinates.
/** \ingroup helper
    \ingroup decorators
    \verbinclude xyzdecorator.py
 */
class IMPCOREEXPORT XYZDecorator: public Decorator
{
 public:
  IMP_DECORATOR(XYZDecorator, Decorator)

  /** Create a decorator with the passed coordinates. */
  static XYZDecorator create(Particle *p,
                             const algebra::Vector3D &v=
                                                 algebra::Vector3D(0,0,0)) {
    p->add_attribute(get_coordinate_key(0),v[0]);
    p->add_attribute(get_coordinate_key(1),v[1]);
    p->add_attribute(get_coordinate_key(2),v[2]);
    return XYZDecorator(p);
  }

  IMP_DECORATOR_GET_SET(x, get_coordinate_key(0), Float, Float);
  IMP_DECORATOR_GET_SET(y, get_coordinate_key(1), Float, Float);
  IMP_DECORATOR_GET_SET(z, get_coordinate_key(2), Float, Float);
  //! set the ith coordinate
  void set_coordinate(unsigned int i, Float v) {
    get_particle()->set_value(get_coordinate_key(i), v);
  }
  //! set all coordinates from a vector
  void set_coordinates(const algebra::Vector3D &v) {
    set_x(v[0]);
    set_y(v[1]);
    set_z(v[2]);
  }

  //! Get the ith coordinate
  Float get_coordinate(int i) const {
    return get_particle()->get_value(get_coordinate_key(i));
  }
  //! Get the ith coordinate derivative
  Float get_coordinate_derivative(int i) const {
    return get_particle()->get_derivative(get_coordinate_key(i));
  }
  //! Add something to the derivative of the ith coordinate
  void add_to_coordinate_derivative(int i, Float v,
                                    DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_coordinate_key(i), v, d);
  }
  //! Add something to the derivative of the coordinates
  void add_to_coordinate_derivatives(const algebra::Vector3D& v,
                                     DerivativeAccumulator &d) {
    add_to_coordinate_derivative(0, v[0], d);
    add_to_coordinate_derivative(1, v[1], d);
    add_to_coordinate_derivative(2, v[2], d);
  }
  //! Get whether the coordinates are optimized
  /** \return true only if all of them are optimized.
    */
  bool get_coordinates_are_optimized() const {
    return get_particle()->get_is_optimized(get_coordinate_key(0))
      && get_particle()->get_is_optimized(get_coordinate_key(1))
      && get_particle()->get_is_optimized(get_coordinate_key(2));
  }
  //! Set whether the coordinates are optimized
  void set_coordinates_are_optimized(bool tf) const {
    get_particle()->set_is_optimized(get_coordinate_key(0), tf);
    get_particle()->set_is_optimized(get_coordinate_key(1), tf);
    get_particle()->set_is_optimized(get_coordinate_key(2), tf);
  }

  //! Get the vector from this particle to another
  algebra::Vector3D get_vector_to(const XYZDecorator &b) const {
    return algebra::Vector3D(b.get_coordinate(0) - get_coordinate(0),
                    b.get_coordinate(1) - get_coordinate(1),
                    b.get_coordinate(2) - get_coordinate(2));
  }

  //! Convert it to a vector.
  /** Somewhat suspect based on wanting a Point/Vector differentiation
      but we don't have points */
  algebra::Vector3D get_coordinates() const {
    return algebra::Vector3D(get_x(), get_y(), get_z());
  }

  //! Get the vector of derivatives.
  /** Somewhat suspect based on wanting a Point/Vector differentiation
      but we don't have points */
  algebra::Vector3D get_derivatives() const {
    return algebra::Vector3D(get_coordinate_derivative(0),
                             get_coordinate_derivative(1),
                             get_coordinate_derivative(2));
  }

  //! Return true if the particle is an instance of an XYZDecorator
  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_coordinate_key(0))
      && p->has_attribute(get_coordinate_key(1))
      && p->has_attribute(get_coordinate_key(2));
  }

  //! Get a vector containing the keys for x,y,z
  /** This is quite handy for initializing movers and things.
   */
  static const FloatKeys& get_xyz_keys();
private:
  static FloatKey get_coordinate_key(unsigned int i) {
    IMP_check(i <3, "Out of range coordinate",
              IndexException);
    return get_xyz_keys()[i];
  }
};

IMP_OUTPUT_OPERATOR(XYZDecorator);

//! Compute the distance between a pair of particles
/** \ingroup helper
    \relates XYZDecorator
 */
IMPCOREEXPORT Float distance(XYZDecorator a, XYZDecorator b);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_XYZ_DECORATOR_H */
