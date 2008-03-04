/**
 *  \file XYZDecorator.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/decorators/XYZDecorator.h"
#include "IMP/random.h"

#include <boost/random/uniform_real.hpp>

#include <cmath>

namespace IMP
{

// These aren't statically initialized, as that way they may be initialized
// before the table that caches them
FloatKeys XYZDecorator::key_(3);

void XYZDecorator::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
  << get_y() << ", " << get_z() <<")";

}

void XYZDecorator::randomize_in_sphere(const Vector3D &center,
                                       float radius)
{
  IMP_check(radius > 0, "Radius in randomize must be postive",
            ValueException("Radius must be positive"));
  Vector3D min(center[0]-radius, center[1]-radius, center[2]-radius);
  Vector3D max(center[0]+radius, center[1]+radius, center[2]+radius);
  float norm;
  do {
    randomize_in_box(min, max);
    norm=0;
    for (int i=0; i< 3; ++i) {
      norm+= square(center[i]-get_coordinate(i));
    }
    norm = std::sqrt(norm);
  } while (norm > radius);
}

void XYZDecorator::randomize_in_box(const Vector3D &min,
                                    const Vector3D &max)
{
  for (unsigned int i=0; i< 3; ++i) {
    IMP_check(min[i] < max[i], "Box for randomize must be non-empty",
              ValueException("Box must be non-empty"));
    ::boost::uniform_real<> rand(min[i], max[i]);
    set_coordinate(i, rand(random_number_generator));
  }
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
