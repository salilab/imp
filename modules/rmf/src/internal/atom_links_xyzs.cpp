/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_xyzs.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadXYZs::HierarchyLoadXYZs(RMF::FileConstHandle f) : ip_factory_(f) {}

void HierarchyLoadXYZs::setup_particle(
    RMF::NodeConstHandle n, kernel::Model *m, kernel::ParticleIndex p,
    const kernel::ParticleIndexes &rigid_bodies) {
  if (!ip_factory_.get_is(n)) return;
  if (!core::XYZ::get_is_setup(m, p)) core::XYZ::setup_particle(m, p);
  /* If there is a rigid body parent set up, add this particle as a child */
  if (!rigid_bodies.empty()) {
    core::RigidBody rb(m, rigid_bodies.back());
    /* For nested rigid bodies, this XYZ particle is *also* the rigid body.
       So don't make ourselves our own child - add to the parent rigid body
       instead. */
    if (rigid_bodies.back() == p) {
      IMP_INTERNAL_CHECK(rigid_bodies.size() >= 2,
                     "Nested rigid body but could not find parent rigid body");
      rb = core::RigidBody(m, rigid_bodies[rigid_bodies.size() - 2]);
    }
    rb.add_member(p);
    if (!ip_factory_.get_is_static(n)) {
      IMP_LOG_VERBOSE("Member particle " << m->get_particle_name(p)
                                         << " is not static" << std::endl);
      rb.set_is_rigid_member(p, false);
    } else {
      IMP_LOG_VERBOSE("Member particle " << m->get_particle_name(p)
                                         << " is static" << std::endl);
      rb.set_is_rigid_member(p, true);
      core::RigidBodyMember(m, p)
          .set_internal_coordinates(get_coordinates(n, ip_factory_));
    }
  }
  link_particle(n, m, p, rigid_bodies);
}

void HierarchyLoadXYZs::link_particle(
    RMF::NodeConstHandle n, kernel::Model *m, kernel::ParticleIndex p,
    const kernel::ParticleIndexes &rigid_bodies) {
  if (!ip_factory_.get_is(n)) return;
  if (rigid_bodies.empty()) {
    global_.push_back(std::make_pair(n.get_id(), p));
  } else {
    if (rigid_bodies.back() == p) return;
    if (core::NonRigidMember::get_is_setup(m, p)) {
      local_.push_back(std::make_pair(n.get_id(), p));
    }
  }
}

void HierarchyLoadXYZs::load(RMF::FileConstHandle fh, Model *m) {
  IMP_FOREACH(Pair pp, global_) {
    IMP_LOG_VERBOSE("Loading global coordinates for "
                    << m->get_particle_name(pp.second) << std::endl);
    algebra::Vector3D rf(get_coordinates(fh.get_node(pp.first), ip_factory_));
    core::XYZ(m, pp.second).set_coordinates(rf);
  }
  IMP_FOREACH(Pair pp, local_) {
    IMP_LOG_VERBOSE("Loading local coordinates for "
                    << m->get_particle_name(pp.second) << std::endl);
    algebra::Vector3D rf(get_coordinates(fh.get_node(pp.first), ip_factory_));
    core::RigidBodyMember(m, pp.second).set_internal_coordinates(rf);
  }
}

HierarchySaveXYZs::HierarchySaveXYZs(RMF::FileHandle f) : ip_factory_(f) {}

void HierarchySaveXYZs::setup_node(
    kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
    const kernel::ParticleIndexes &rigid_bodies) {
  if (!core::XYZ::get_is_setup(m, p)) return;
  if (rigid_bodies.empty()) {
    global_.push_back(std::make_pair(n.get_id(), p));
  } else {
    if (core::XYZR::get_is_setup(m, p) && rigid_bodies.back() == p) {
      ip_factory_.get(n).set_static_coordinates(RMF::Vector3(0, 0, 0));
    } else if (core::RigidMember::get_is_setup(m, p)) {
      copy_to_static_particle(
          core::RigidBodyMember(m, p).get_internal_coordinates(), n,
          ip_factory_);
    } else if (core::NonRigidMember::get_is_setup(m, p)) {
      local_.push_back(std::make_pair(n.get_id(), p));
    } else {
      IMP_FAILURE("not sure why I am here");
    }
  }
}

void HierarchySaveXYZs::save(kernel::Model *m, RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, global_) {
    copy_to_frame_particle(core::XYZ(m, pp.second).get_coordinates(),
                           fh.get_node(pp.first), ip_factory_);
  }
  IMP_FOREACH(Pair pp, local_) {
    copy_to_frame_particle(
        core::RigidBodyMember(m, pp.second).get_internal_coordinates(),
        fh.get_node(pp.first), ip_factory_);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
