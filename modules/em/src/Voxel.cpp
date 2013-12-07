/**
 *  \file Voxel.cpp   \brief Voxel decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/Voxel.h>

IMPEM_BEGIN_NAMESPACE

void Voxel::show(std::ostream &out) const {
  algebra::VectorD<5> v;
  kernel::Particle *p = get_particle();
  core::XYZR xyzr(p);
  v[0] = xyzr.get_x();
  v[1] = xyzr.get_y();
  v[2] = xyzr.get_z();
  v[3] = xyzr.get_radius();  // p->get_value(core::XYZR::get_radius_key());
  v[4] = p->get_value(get_density_key());
  out << "(" << algebra::commas_io(v) << ")";
}

const FloatKey Voxel::get_density_key() {
  static FloatKey density = FloatKey("density_val");
  return density;
}

const FloatKeys Voxel::get_keys() {
  static FloatKeys keys = core::XYZ::get_xyz_keys();
  keys.push_back(core::XYZR::get_radius_key());
  keys.push_back(Voxel::get_density_key());
  return keys;
}

IMPEM_END_NAMESPACE
