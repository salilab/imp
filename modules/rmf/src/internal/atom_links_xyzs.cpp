/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_xyzs.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadXYZs::HierarchyLoadXYZs(RMF::FileConstHandle f)
  : reference_frame_factory_(f), ip_factory_(f) {
  // backwards compat
  RMF::Category cat = f.get_category("IMP");
  rb_index_key_ = f.get_key(cat, "rigid body", RMF::IntTraits());
}

void HierarchyLoadXYZs::setup_particle(
    RMF::NodeConstHandle n, Model *m, ParticleIndex p,
    const ParticleIndexes &rigid_bodies) {
  if (!ip_factory_.get_is(n)) return;
  if (!core::XYZ::get_is_setup(m, p)) core::XYZ::setup_particle(m, p);
  /* If there is a rigid body parent set up, add this particle as a child
     (unless it's an old-style rigid body, in which case this has been
     done already) */
  if (!rigid_bodies.empty()
      && !(rigid_bodies.size()==1 && rigid_bodies.back() == p)
      && !n.get_has_value(rb_index_key_)) {
    core::RigidBody rb(m, rigid_bodies.back());
    /* For nested rigid bodies, this XYZ particle is *also* the rigid body.
       So don't make ourselves our own child - add to the parent rigid body
       instead. */
    if (rigid_bodies.back() == p) {
      IMP_INTERNAL_CHECK(rigid_bodies.size() >= 2,
                     "Nested rigid body " << m->get_particle_name(p)
                     << " but could not find parent rigid body");
      rb = core::RigidBody(m, rigid_bodies[rigid_bodies.size() - 2]);
    }
    rb.add_member(p);
    if (reference_frame_factory_.get_is(n)
        && !reference_frame_factory_.get_is_static(n)) {
      IMP_LOG_VERBOSE("Member particle " << m->get_particle_name(p)
                      << " is not static and is also a rigid body"
                      << std::endl);
      rb.set_is_rigid_member(p, false);
    } else if (!ip_factory_.get_is_static(n)) {
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
    RMF::NodeConstHandle n, Model *m, ParticleIndex p,
    const ParticleIndexes &rigid_bodies) {
  if (!ip_factory_.get_is(n)) return;
  if (rigid_bodies.empty()) {
    global_.push_back(std::make_pair(n.get_id(), p));
  } else {
    if (rigid_bodies.back() == p) return;
    if (core::NonRigidMember::get_is_setup(m, p)) {
      local_.push_back(std::make_pair(n.get_id(), p));
    } else if (n.get_has_value(rb_index_key_)
               && core::RigidMember::get_is_setup(m, p)) {
      // backwards compat: need to read coordinates of rigid members in order
      // to set the reference frame of the rigid body later
      global_.push_back(std::make_pair(n.get_id(), p));
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
    Model *m, ParticleIndex p, RMF::NodeHandle n,
    const ParticleIndexes &rigid_bodies) {
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
      /* Treat a particle that is a child of a rigid body but not a member
         as a nonrigid member. This results in tidier RMF files than using the
         external rigid body mechanism (which duplicates the rigid body's
         reference frame for every particle in the body). The (minor) drawback
         is that when the RMF is read back in, these particles will become
         "real" nonrigid members. */
      rigid_nonmember_.push_back(std::make_pair(n.get_id(),
                           ParticleIndexPair(p, rigid_bodies.back())));
    }
  }
}

void HierarchySaveXYZs::save(Model *m, RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, global_) {
    copy_to_frame_particle(core::XYZ(m, pp.second).get_coordinates(),
                           fh.get_node(pp.first), ip_factory_);
  }
  IMP_FOREACH(Triplet pp, rigid_nonmember_) {
    /* Transform global coordinates to those relative to the "parent"
       rigid body */
    algebra::Vector3D global_coord
                          = core::XYZ(m, pp.second[0]).get_coordinates();
    algebra::ReferenceFrame3D rf
                      = core::RigidBody(m, pp.second[1]).get_reference_frame();
    copy_to_frame_particle(
                   rf.get_transformation_from().get_transformed(global_coord),
                   fh.get_node(pp.first), ip_factory_);
  }
  IMP_FOREACH(Pair pp, local_) {
    copy_to_frame_particle(
        core::RigidBodyMember(m, pp.second).get_internal_coordinates(),
        fh.get_node(pp.first), ip_factory_);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
