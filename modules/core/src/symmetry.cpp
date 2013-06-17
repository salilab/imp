/**
 *  \file symmetry.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/symmetry.h>
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

ParticleIndexKey Reference::get_reference_key() {
  static ParticleIndexKey k("reference particle");
  return k;
}

void Reference::show(std::ostream &out) const {
  out << get_particle()->get_name() << " references "
      << get_reference_particle()->get_name();
}

TransformationSymmetry::TransformationSymmetry(
    const algebra::Transformation3D &t) {
  t_ = t;
}

void TransformationSymmetry::apply_index(Model *m, ParticleIndex pi) const {
  set_was_used(true);
  if (RigidBody::particle_is_instance(m, pi)) {
    RigidBody rrb(Reference(m, pi).get_reference_particle());
    RigidBody rb(m, pi);
    // We do the non-lazy version in order as it is hard
    // for the dependency checker to get the dependencies right
    // Is it really? We should check this.
    rb.set_reference_frame(algebra::ReferenceFrame3D(
        t_ * rrb.get_reference_frame().get_transformation_to()));
  } else {
    XYZ rd(Reference(m, pi).get_reference_particle());
    XYZ d(m, pi);
    d.set_coordinates(t_.get_transformed(rd.get_coordinates()));
  }
}

ModelObjectsTemp TransformationSymmetry::do_get_inputs(Model *m,
                                           const ParticleIndexes &pis) const {
  ModelObjectsTemp ret = IMP::kernel::get_particles(m, pis);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    if (RigidBody::particle_is_instance(m, pis[i])) {
      ret += RigidBody(m, pis[i]).get_members();
    }
    ret.push_back(Reference(m, pis[i]).get_reference_particle());
  }
  return ret;
}

ModelObjectsTemp TransformationSymmetry::do_get_outputs(Model *m,
                                           const ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

TransformationAndReflectionSymmetry::TransformationAndReflectionSymmetry(
    const algebra::Transformation3D &t, const algebra::Reflection3D &r) {
  t_ = t;
  r_ = r;
}

void TransformationAndReflectionSymmetry
::apply_index(Model *m, ParticleIndex pi) const {
  IMP_USAGE_CHECK(!RigidBody::particle_is_instance(m, pi),
                  "Particle must not be a rigid body particle");
  set_was_used(true);
  XYZ rd(Reference(m, pi).get_reference_particle());
  XYZ d(m, pi);
  d.set_coordinates(t_.get_transformed(r_.get_reflected(rd.get_coordinates())));
}

ModelObjectsTemp
TransformationAndReflectionSymmetry::do_get_inputs(Model *m,
                                      const ParticleIndexes &pis) const {
  ModelObjectsTemp ret(2 * pis.size());
  for (unsigned int i = 0; i < pis.size(); ++i) {
    ret[2 * i + 0] = m->get_particle(pis[i]);
    ret[2 * i + 0] = Reference(m, pis[i]).get_reference_particle();
  }
  return ret;
}

ModelObjectsTemp TransformationAndReflectionSymmetry::do_get_outputs(
    Model *m,
                                           const ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
