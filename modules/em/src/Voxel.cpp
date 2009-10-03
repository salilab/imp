/**
 *  \file Voxel.cpp   \brief Voxel decorator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/Voxel.h>

IMPEM_BEGIN_NAMESPACE

void Voxel::show(std::ostream &out) const
{
  algebra::VectorD<4> v;
  Particle *p=get_particle();
  core::XYZ xyz(p);
  v[0]=xyz.get_x();
  v[1]=xyz.get_y();
  v[2]=xyz.get_z();
  v[3]=p->get_value(get_density_key());
  out << "(" <<algebra::commas_io(v)<<")";
}

const FloatKey Voxel::get_density_key() {
  static FloatKey density=FloatKey("density_val");
  return density;
}

const FloatKeys Voxel::get_keys() {
  static FloatKeys keys=core::XYZ::get_xyz_keys();
  keys.push_back(Voxel::get_density_key());
  return keys;
}


IMPEM_END_NAMESPACE
