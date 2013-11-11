/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_local_coordinates.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadLocalCoordinates::HierarchyLoadLocalCoordinates(
    RMF::FileConstHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {}

bool HierarchyLoadLocalCoordinates::setup_particle(
    RMF::NodeConstHandle n, unsigned int cstate, kernel::Model *m,
    kernel::ParticleIndex p, const kernel::ParticleIndexes &rigid_bodies) {
  if (rigid_bodies.empty()) return false;
  link_particle(n, cstate, m, p, rigid_bodies);
  if (cstate & FRAME_XYZ) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                  << " is a local rigid member." << std::endl);
    core::XYZ::setup_particle(m, p);
    core::RigidBody(m, rigid_bodies.back()).add_non_rigid_member(p);
  }
  if (cstate & FRAME_RB) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                  << " is a local non-rigid member." << std::endl);
    core::RigidBody::setup_particle(m, p, algebra::ReferenceFrame3D());
    core::RigidBody(m, rigid_bodies.back()).add_non_rigid_member(p);
  }
}

HierarchyLoadLocalCoordinates::Type
HierarchyLoadLocalCoordinates::link_particle(
    RMF::NodeConstHandle n, unsigned int cstate, kernel::Model *m,
    kernel::ParticleIndex p, const kernel::ParticleIndexes &rigid_bodies) {
  // check at frame 0
  if (rigid_bodies.empty()) return NONE;
  if (cstate & FRAME_RB) {
    rigid_bodies_.push_back(std::make_pair(n.get_id(), p));
    return RIGID_BODY;
  } else if (cstate & FRAME_XYZ) {
    xyzs_.push_back(std::make_pair(n.get_id(), p));
    return PARTICLE;
  }
}

void HierarchyLoadLocalCoordinates::load(RMF::FileConstHandle fh,
                                         kernel::Model *m) {
  IMP_FOREACH(Pair pp, xyzs_) {
    algebra::Vector3D v =
        get_coordinates(fh.get_node(pp.first), intermediate_particle_factory_);
    core::RigidBodyMember(m, pp.second).set_internal_coordinates(v);
  }
  IMP_FOREACH(Pair pp, rigid_bodies_) {
    algebra::Transformation3D rf(
        get_transformation(fh.get_node(pp.first), reference_frame_factory_));
    core::RigidBodyMember(m, pp.second).set_internal_transformation(rf);
  }
}

HierarchySaveLocalCoordinates::HierarchySaveLocalCoordinates(RMF::FileHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {}

bool HierarchySaveLocalCoordinates::setup_node(
    kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
    kernel::ParticleIndexes rigid_bodies) {
  // assume not static
  if (rigid_bodies.empty()) return false;
  if (!core::NonRigidMember::get_is_setup(m, p)) return false;
  if (core::RigidBody::get_is_setup(m, p)) {
    rigid_bodies_.push_back(std::make_pair(n.get_id(), p));
  } else if (core::XYZ::get_is_setup(m, p)) {
    xyzs_.push_back(std::make_pair(n.get_id(), p));
  }
  return true;
}

void HierarchySaveLocalCoordinates::save(Model *m, RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, xyzs_) {
    copy_to_particle(
        core::RigidBodyMember(m, pp.second).get_internal_coordinates(),
        fh.get_node(pp.first), intermediate_particle_factory_);
  }
  IMP_FOREACH(Pair pp, rigid_bodies_) {
    copy_to_reference_frame(
        core::RigidBodyMember(m, pp.second).get_internal_transformation(),
        fh.get_node(pp.first), reference_frame_factory_);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
