/**
 *  \file XYZ.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

void XYZ::show(std::ostream &out) const {
  out << "(" << algebra::commas_io(get_coordinates()) << ")";
}

const FloatKeys &XYZ::get_xyz_keys() {
  static FloatKeys fks(IMP::internal::xyzr_keys, IMP::internal::xyzr_keys + 3);
  return fks;
}

void transform(XYZ a, const algebra::Transformation3D &tr) {
  IMP_USAGE_CHECK(!RigidBody::get_is_setup(a),
                  "Python is calling the wrong function");
  a.set_coordinates(tr.get_transformed(a.get_coordinates()));
}

double get_dihedral(XYZ a, XYZ b, XYZ c, XYZ d) {
  return internal::dihedral(a.get_coordinates(), b.get_coordinates(),
                            c.get_coordinates(), d.get_coordinates(), nullptr,
                            nullptr, nullptr, nullptr);
}

IMPCORE_END_NAMESPACE
