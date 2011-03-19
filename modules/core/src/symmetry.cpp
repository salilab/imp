/**
 *  \file symmetry.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */


#include <IMP/core/symmetry.h>
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

ParticleKey Reference::get_reference_key() {
  static ParticleKey k("reference particle");
  return k;
}

void Reference::show(std::ostream &out) const {
  out << get_particle()->get_name() << " references "
      << get_reference_particle()->get_name();
}

TransformationSymmetry::TransformationSymmetry(
       const algebra::Transformation3D &t)
{
  t_=t;
}

void TransformationSymmetry::apply(Particle *p) const
{
  if (RigidBody::particle_is_instance(p)) {
    RigidBody rrb(Reference(p).get_reference_particle());
    RigidBody rb(p);
    rb.set_reference_frame(
    algebra::ReferenceFrame3D(t_*rrb.get_reference_frame()
                              .get_transformation_to()));
  } else {
    XYZ rd(Reference(p).get_reference_particle());
    XYZ d(p);
    d.set_coordinates(t_.get_transformed(rd.get_coordinates()));
  }
}

ParticlesTemp TransformationSymmetry::get_input_particles(Particle *p) const {
  ParticlesTemp ret;
  ret.push_back(p);
  ret.push_back(Reference(p).get_reference_particle());
  return ret;
}

ParticlesTemp TransformationSymmetry::get_output_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}

ContainersTemp TransformationSymmetry::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ContainersTemp TransformationSymmetry::get_output_containers(Particle *) const {
  return ContainersTemp();
}

void TransformationSymmetry::do_show(std::ostream &out) const
{
  out<<"transformation: "<< t_ << std::endl;
}



TransformationAndReflectionSymmetry::TransformationAndReflectionSymmetry(
     const algebra::Transformation3D &t,
     const algebra::Reflection3D &r)
{
  t_=t;
  r_=r;
}

void TransformationAndReflectionSymmetry::apply(Particle *p) const
{
  IMP_USAGE_CHECK(!RigidBody::particle_is_instance(p),
                  "Particle must not be a rigid body particle");
  XYZ rd(Reference(p).get_reference_particle());
  XYZ d(p);
  d.set_coordinates(t_.get_transformed(r_.get_reflected(rd.get_coordinates())));
}

ParticlesTemp TransformationAndReflectionSymmetry
::get_input_particles(Particle *p) const {
  ParticlesTemp ret;
  ret.push_back(p);
  ret.push_back(Reference(p).get_reference_particle());
  return ret;
}

ParticlesTemp TransformationAndReflectionSymmetry
::get_output_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}

ContainersTemp TransformationAndReflectionSymmetry
::get_input_containers(Particle *) const {
  return ContainersTemp();
}

ContainersTemp TransformationAndReflectionSymmetry
::get_output_containers(Particle *) const {
  return ContainersTemp();
}

void TransformationAndReflectionSymmetry::do_show(std::ostream &out) const
{
  out<<"transformation: "<< t_ << std::endl;
}

IMPCORE_END_NAMESPACE
