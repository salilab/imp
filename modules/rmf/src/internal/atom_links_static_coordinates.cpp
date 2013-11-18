/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_static_coordinates.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadStaticCoordinates::HierarchyLoadStaticCoordinates(
    RMF::FileConstHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {}

bool HierarchyLoadStaticCoordinates::setup_particle(
    RMF::NodeConstHandle n, unsigned int cstate, kernel::Model *m,
    kernel::ParticleIndex p, const kernel::ParticleIndexes &rigid_bodies) {
  if (rigid_bodies.empty()) {
    return false;
  }
  kernel::ParticleIndex rbp = rigid_bodies.back();
  core::RigidBody rb(m, rbp);
  if (cstate & STATIC_RB) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                              << " is a static rigid member." << std::endl);
    rb.add_member(p);
    core::RigidBodyMember rbm(m, p);
    rbm.set_internal_transformation(
        get_transformation(n, reference_frame_factory_));
  } else if (cstate & STATIC_XYZ) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                              << " is a static xyz member." << std::endl);
    core::XYZ d = core::XYZ::setup_particle(m, p);
    rb.add_member(p);
    core::RigidBodyMember rbm(m, p);
    rbm.set_internal_coordinates(
        get_coordinates(n, intermediate_particle_factory_));
  }
  return true;
}

HierarchySaveStaticCoordinates::HierarchySaveStaticCoordinates(
    RMF::FileHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {}

bool HierarchySaveStaticCoordinates::setup_node(
    kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
    kernel::ParticleIndexes rigid_bodies) {

  if (core::RigidBody::get_is_setup(m, p) && core::XYZR::get_is_setup(m, p)) {
    RMF::Floats zero(3, 0.0);
    intermediate_particle_factory_.get(n).set_coordinates(zero);
  }
  if (rigid_bodies.empty()) {
    return false;
  }
  IMP_USAGE_CHECK(core::RigidBodyMember::get_is_setup(m, p),
                  "All descendents of a rigid body in the hierarchy must be"
                      << " members.");
  // check that back() matches my rigid body
  core::RigidBodyMember rbm(m, p);
  IMP_USAGE_CHECK(
      rbm.get_rigid_body().get_particle_index() == rigid_bodies.back(),
      "All direct descendents of a rigid"
          << " body in the hierarchy must be members of it.");
  if (core::RigidMember::get_is_setup(m, p)) {
    if (core::RigidBody::get_is_setup(m, p)) {
      copy_to_static_reference_frame(rbm.get_internal_transformation(), n,
                                     reference_frame_factory_);
    } else {
      copy_to_static_particle(rbm.get_internal_coordinates(), n,
                              intermediate_particle_factory_);
    }
  }
  return true;
}

IMPRMF_END_INTERNAL_NAMESPACE
