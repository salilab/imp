/**
 *  \file XYZDecorator.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <sstream>
#include <cmath>

#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{

// These aren't statically initialized, as that way they may be initialized
// before the table that caches them
FloatKey XYZDecorator::key_[3];

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

namespace {
  template <class T>
  T d(T a, T b){T d=a-b; return d*d;}
}

Float distance(XYZDecorator a, XYZDecorator b)
{
  double d2= d(a.get_x(), b.get_x()) + d(a.get_y(), b.get_y())
    + d(a.get_z(), b.get_z());
  return std::sqrt(d2);
}

} // namespace IMP
