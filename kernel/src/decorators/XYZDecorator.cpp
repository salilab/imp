/**
 *  \file XYZDecorator.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/decorators/XYZDecorator.h"


#include <cmath>

IMP_BEGIN_NAMESPACE

// These aren't statically initialized, as that way they may be initialized
// before the table that caches them
FloatKeys XYZDecorator::key_(3);

void XYZDecorator::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
  << get_y() << ", " << get_z() <<")";

}

IMP_DECORATOR_INITIALIZE(XYZDecorator, DecoratorBase,
                         {
                         key_[0] = FloatKey("x");
                         key_[1] = FloatKey("y");
                         key_[2] = FloatKey("z");
                         })

Float distance(XYZDecorator a, XYZDecorator b)
{
  return (a.get_vector()-b.get_vector()).get_magnitude();
}

IMP_END_NAMESPACE
