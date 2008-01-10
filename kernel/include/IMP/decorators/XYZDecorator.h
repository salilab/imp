/**
 *  \file XYZDecorator.h     \brief Simple xyz decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_XYZ_DECORATOR_H
#define __IMP_XYZ_DECORATOR_H

#include <vector>
#include <deque>
#include <limits>

#include "../Particle.h"
#include "../Model.h"
#include "../DecoratorBase.h"
#include "../Vector3D.h"
#include "utility.h"

namespace IMP
{

//! A a decorator for a particle with x,y,z coordinates.
class IMPDLLEXPORT XYZDecorator: public DecoratorBase
{
  IMP_DECORATOR(XYZDecorator, DecoratorBase,
                {
                  return p->has_attribute(key_[0])
                    && p->has_attribute(key_[1])
                    && p->has_attribute(key_[2]);
                    },
                { p->add_attribute(key_[0],
                                   std::numeric_limits<float>::infinity());
                  p->add_attribute(key_[1],
                                   std::numeric_limits<float>::infinity());
                  p->add_attribute(key_[2],
                                   std::numeric_limits<float>::infinity());
                });

protected:
  static FloatKey key_[3];

public:
  IMP_DECORATOR_GET_SET(x, key_[0], Float, Float);
  IMP_DECORATOR_GET_SET(y, key_[1], Float, Float);
  IMP_DECORATOR_GET_SET(z, key_[2], Float, Float);
  //! set the ith coordinate
  void set_coordinate(unsigned int i, Float v) {
    get_particle()->set_value(get_coordinate_key(i), v);
  }
  //! Get the ith coordinate
  Float get_coordinate(int i) const {
    return get_particle()->get_value(get_coordinate_key(i));
  }
  //! Add something to the derivative of the ith coordinate
  void add_to_coordinate_derivative(int i, Float v, 
                                    DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_coordinate_key(i), v, d);
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
  Vector3D get_vector_to(const XYZDecorator &b) const {
    return Vector3D(b.get_coordinate(0) - get_coordinate(0),
                    b.get_coordinate(1) - get_coordinate(1),
                    b.get_coordinate(2) - get_coordinate(2));
  }
protected:
  static FloatKey get_coordinate_key(unsigned int i) {
    IMP_check(i <3, "Out of range coordinate",
              IndexException("Out of range coordinate"));
    return key_[i];
  }
};

IMP_OUTPUT_OPERATOR(XYZDecorator);

//! Compute the distance between a pair of particles
IMPDLLEXPORT Float distance(XYZDecorator a, XYZDecorator b);

} // namespace IMP

#endif  /* __IMP_XYZ_DECORATOR_H */
