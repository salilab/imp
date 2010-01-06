/**
 *  \file XYZ.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/XYZ.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

void XYZ::show(std::ostream &out) const
{
  out << "(" <<algebra::commas_io(get_coordinates())<<")";

}

const FloatKeys& XYZ::get_xyz_keys() {
  static FloatKeys fks(IMP::internal::xyzr_keys,
                       IMP::internal::xyzr_keys+3);
  return fks;
}

IMPCORE_END_NAMESPACE
