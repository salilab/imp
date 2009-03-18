/**
 *  \file XYZDecorator.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/XYZDecorator.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

void XYZDecorator::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" << get_x()<< ", "
  << get_y() << ", " << get_z() <<")";

}

const FloatKeys& XYZDecorator::get_xyz_keys() {
  static FloatKey ks[3]={FloatKey("x"), FloatKey("y"), FloatKey("z")};
  static FloatKeys fks(ks, ks+3);
  return fks;
}

Float distance(XYZDecorator a, XYZDecorator b)
{
  return algebra::distance(a.get_coordinates(),b.get_coordinates());
}

IMPCORE_END_NAMESPACE
