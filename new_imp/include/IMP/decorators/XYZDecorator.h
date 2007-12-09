/**
 *  \file XYZDecorator.h     \brief Simple xyz decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
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
#include "utility.h"

namespace IMP
{

  //! A a decorator for a particle with x,y,z coordinates.
  class IMPDLLEXPORT XYZDecorator: public DecoratorBase
  {
    IMP_DECORATOR(XYZDecorator, DecoratorBase,
                  return p->has_attribute(x_key_) && p->has_attribute(y_key_)
                  && p->has_attribute(z_key_),
                  { p->add_attribute(x_key_,
                                     std::numeric_limits<float>::infinity());
                    p->add_attribute(y_key_,
                                     std::numeric_limits<float>::infinity());
                    p->add_attribute(z_key_,
                                     std::numeric_limits<float>::infinity());
                  });

  protected:
    static FloatKey x_key_;
    static FloatKey y_key_;
    static FloatKey z_key_;

  public:
    IMP_DECORATOR_GET_SET(x, x_key_, Float, Float);
    IMP_DECORATOR_GET_SET(y, y_key_, Float, Float);
    IMP_DECORATOR_GET_SET(z, z_key_, Float, Float);

    void set_coordinate(int i, Float v) {
      switch (i) {
      case 0: set_x(v); break;
      case 1: set_y(v); break;
      case 2: set_z(v); break;
      default:
        throw IndexException("invalid coordinate");
      }
    }
    Float get_coordinate(int i) const {
      switch (i) {
      case 0: return get_x();
      case 1: return get_y();
      case 2: return get_z();
      default:
        throw IndexException("invalid coordinate");
        return 0;
      }
    }

  };

  IMP_OUTPUT_OPERATOR(XYZDecorator);

  //! Compute the distance between a pair of particles
  IMPDLLEXPORT Float distance(XYZDecorator a, XYZDecorator b);

} // namespace IMP

#endif  /* __IMP_XYZ_DECORATOR_H */
