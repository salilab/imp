/**
 *  \file XYZ.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/XYZ.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

void XYZ::show(std::ostream &out, std::string prefix) const
{
  out << prefix << "(" <<algebra::commas_io(get_coordinates())<<")";

}

Float distance(XYZ a, XYZ b)
{
  return algebra::distance(a.get_coordinates(),b.get_coordinates());
}

IMPCORE_END_NAMESPACE
