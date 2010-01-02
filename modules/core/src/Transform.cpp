/**
 *  \file TransformSingletonParticle.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */


#include <IMP/core/Transform.h>
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

Transform::Transform(
     const algebra::Transformation3D &t,
     bool ignore_non_xyz)
{
  t_=t;
  ignore_non_xyz_=ignore_non_xyz_;
}

void Transform::apply(Particle *p) const
{
  if (!XYZ::particle_is_instance(p)) {
    IMP_INTERNAL_CHECK(ignore_non_xyz_,
                       "The particle does not have XYZ attributes");
    return;
  }
  XYZ xyz = XYZ(p);
  xyz.set_coordinates(t_.transform(xyz.get_coordinates()));
}

ParticlesList Transform::get_interacting_particles(Particle *) const {
  return ParticlesList();
}

ParticlesTemp Transform::get_input_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}

ParticlesTemp Transform::get_output_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}

ContainersTemp Transform::get_input_containers(Particle *p) const {
  return ContainersTemp();
}

ContainersTemp Transform::get_output_containers(Particle *p) const {
  return ContainersTemp();
}

void Transform::show(std::ostream &out) const
{
  out<<"TransformParticle :";
  t_.show(out);
  out<<std::endl;
}

IMPCORE_END_NAMESPACE
