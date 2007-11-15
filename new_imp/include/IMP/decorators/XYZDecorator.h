/**
 *  \file XYZDecorator.h     \brief Simple xyz decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_XYZ_DECORATOR_H
#define __IMP_XYZ_DECORATOR_H

#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include <vector>
#include <deque>
#include <limits>

namespace IMP
{

//! A single name decorator.
class IMPDLLEXPORT XYZDecorator
{
public:
  IMP_DECORATOR(XYZDecorator,
                p->has_attribute(x_key_) && p->has_attribute(y_key_)
                && p->has_attribute(z_key_),
                { p->add_attribute(x_key_,
                                   std::numeric_limits<float>::infinity());
                  p->add_attribute(y_key_,
                                   std::numeric_limits<float>::infinity());
                  p->add_attribute(z_key_,
                                   std::numeric_limits<float>::infinity()
                });

protected:
  static bool keys_initialized_;
  static FloatKey x_key_;
  static FloatKey y_key_;
  static FloatKey z_key_;

public:
  IMP_DECORATOR_GET_SET(x, x_key_, Float, Float);
  IMP_DECORATOR_GET_SET(y, y_key_, Float, Float);
  IMP_DECORATOR_GET_SET(z, z_key_, Float, Float);

};

IMP_OUTPUT_OPERATOR(XYZDecorator);

} // namespace IMP

#endif  /* __IMP_XYZ_DECORATOR_H */
