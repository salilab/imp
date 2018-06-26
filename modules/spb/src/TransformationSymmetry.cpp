/**
 *  \file TransformationSymmetry.cpp
 *  \brief Transform a particle
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include "IMP/spb/TransformationSymmetry.h"
#include <IMP/core.h>
#include <IMP/isd.h>

IMPSPB_BEGIN_NAMESPACE

TransformationSymmetry::TransformationSymmetry(algebra::Transformation3D t,
                                               Particle *px, Particle *py,
                                               Particle *pz) {
  t_ = t;
  px_ = px;
  py_ = py;
  pz_ = pz;
}

algebra::Vector3D TransformationSymmetry::get_vector(
    algebra::Vector3D center) const {
  Float sx = isd::Scale(px_).get_scale();
  Float sy = isd::Scale(py_).get_scale();
  Float sz = isd::Scale(pz_).get_scale();
  algebra::Vector3D newcenter =
      algebra::Vector3D(center[0] * sx, center[1] * sy, center[2] * sz);
  return newcenter;
}

algebra::Transformation3D TransformationSymmetry::get_transformation() const {
  algebra::Rotation3D rr = t_.get_rotation();
  algebra::Vector3D tt = t_.get_translation();
  algebra::Transformation3D newtrans(rr, get_vector(tt));
  return newtrans;
}

void TransformationSymmetry::apply_index(IMP::Model *m,
                                         IMP::ParticleIndex pi) const {
  set_was_used(true);

  algebra::Transformation3D t = get_transformation();
  if (core::RigidBody::get_is_setup(m, pi)) {
    core::RigidBody rrb(
        core::Reference(m->get_particle(pi)).get_reference_particle());
    core::RigidBody rb(m->get_particle(pi));
    // We do the non-lazy version in order as it is hard
    // for the dependency checker to get the dependencies right
    // Is it really? We should check this.
    rb.set_reference_frame(algebra::ReferenceFrame3D(
        t * rrb.get_reference_frame().get_transformation_to()));
    // std::cout << pi <<" " << rb.get_reference_frame() << std::endl ;

  } else {
    core::XYZ rd(core::Reference(m->get_particle(pi)).get_reference_particle());
    core::XYZ d(m->get_particle(pi));
    d.set_coordinates(t.get_transformed(rd.get_coordinates()));
  }
}

IMP::ModelObjectsTemp TransformationSymmetry::do_get_inputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  ParticlesTemp ret;

  for (unsigned int j = 0; j < pis.size(); j++)  // index i was already used
  {
    ret.push_back(m->get_particle(pis[j]));
    if (core::RigidBody::get_is_setup(m, pis[j])) {
      for (unsigned int i = 0;
           i < core::RigidBody(m->get_particle(pis[j])).get_number_of_members();
           ++i) {
        ret.push_back(core::RigidBody(m->get_particle(pis[j])).get_member(i));
      }
    }
    ret.push_back(
        core::Reference(m->get_particle(pis[j])).get_reference_particle());
  }
  ret.push_back(px_);
  ret.push_back(py_);
  ret.push_back(pz_);
  return ret;
}

IMP::ModelObjectsTemp TransformationSymmetry::do_get_outputs(
    IMP::Model *m, const IMP::ParticleIndexes &pis) const {
  ParticlesTemp ret;

  for (unsigned int j = 0; j < pis.size(); j++)  // index i was already used
  {
    ret.push_back(m->get_particle(pis[j]));

    if (core::RigidBody::get_is_setup(m, pis[j])) {
      for (unsigned int i = 0;
           i < core::RigidBody(m->get_particle(pis[j])).get_number_of_members();
           ++i) {
        ret.push_back(core::RigidBody(m->get_particle(pis[j])).get_member(i));
      }
    }
  }
  ret.push_back(px_);
  ret.push_back(py_);
  ret.push_back(pz_);
  return ret;
}

void TransformationSymmetry::show(std::ostream &out) const {
  out << "transformation: " << get_transformation() << std::endl;
}

IMPSPB_END_NAMESPACE
