/**
 *  \file IMP/em/Voxel.h     \brief voxel decorator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_VOXEL_H
#define IMPEM_VOXEL_H

#include <IMP/em/em_config.h>
#include <IMP/Decorator.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>

IMPEM_BEGIN_NAMESPACE

//! A a decorator for a particle that stores data on a density voxel , such as:
//! position (x,y,z) attributes and density attributes
/**
    \ingroup decorators
 */
class IMPEMEXPORT Voxel: public Decorator
{
  static void do_setup_particle(Model *m, ParticleIndex pi,
                  const algebra::Vector3D &position,
                  Float radius,
                  Float density) {
    core::XYZR::setup_particle(m, pi, algebra::Sphere3D(position,radius));
    m->add_attribute(get_density_key(),pi, density,false);
  }
 public:

  static const FloatKey get_density_key();
  static const FloatKeys get_keys();

  IMP_DECORATOR_METHODS(Voxel, Decorator);
  IMP_DECORATOR_SETUP_3(Voxel, algebra::Vector3D, position,
                        Float, radius,
                        Float, density);

  IMP_DECORATOR_GET_SET(density, get_density_key(), Float, Float);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    if (!core::XYZ::get_is_setup(m, pi)) return false;
    return m->get_has_attribute(get_density_key(), pi);
  }
};

IMP_DECORATORS(Voxel, Voxels, Particles);

IMPEM_END_NAMESPACE

#endif  /* IMPEM_VOXEL_H */
