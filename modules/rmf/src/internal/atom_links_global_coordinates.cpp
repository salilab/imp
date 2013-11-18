/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_global_coordinates.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadGlobalCoordinates::HierarchyLoadGlobalCoordinates(
    RMF::FileConstHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {
  RMF::Category cat = f.get_category("IMP");
  key_ = f.get_key(cat, "rigid body", RMF::IntTraits());
  non_rigid_key_ = f.get_key(cat, "non rigid", RMF::IntTraits());
}

bool HierarchyLoadGlobalCoordinates::setup_particle(
    RMF::NodeConstHandle n, unsigned int cstate, kernel::Model *m,
    kernel::ParticleIndex p, const kernel::ParticleIndexes &rigid_bodies) {
  if (!rigid_bodies.empty()) return false;
  if (cstate & FRAME_RB) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                              << " is a global rigid body." << std::endl);
    core::RigidBody::setup_particle(m, p, algebra::ReferenceFrame3D());
  } else if (cstate & FRAME_XYZ) {
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p)
                              << " is a global XYZ particle." << std::endl);
    if (!core::XYZ::get_is_setup(m, p)) {
      core::XYZ::setup_particle(m, p);
    }
  }
  if (n.get_has_value(key_)) {
    int rb = n.get_value(key_);
    if (rigid_body_compositions_[rb].rb == kernel::ParticleIndex()) {
      kernel::ParticleIndex pi = m->add_particle("RB%1%");
      rigid_body_compositions_[rb].initialized = false;
      rigid_body_compositions_[rb].rb = pi;
      core::RigidBody::setup_particle(m, pi, algebra::ReferenceFrame3D());
    }
    core::RigidBody arb(m, rigid_body_compositions_.find(rb)->second.rb);
    if (n.get_has_value(non_rigid_key_)) {
      arb.add_non_rigid_member(p);
    } else {
      arb.add_member(p);
    }
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p) << " is in rigid body "
                              << std::endl);

  }
  link_particle(n, cstate, m, p, rigid_bodies);
  return true;
}

bool HierarchyLoadGlobalCoordinates::link_particle(
    RMF::NodeConstHandle n, unsigned int cstate, kernel::Model *m,
    kernel::ParticleIndex p, const kernel::ParticleIndexes &rigid_bodies) {
  if (!rigid_bodies.empty()) return false;
  if (cstate & FRAME_RB) {
    rigid_bodies_.push_back(std::make_pair(n.get_id(), p));
  } else if (cstate & FRAME_XYZ) {
    xyzs_.push_back(std::make_pair(n.get_id(), p));
  }
  if (n.get_has_value(key_)) {
    int rb = n.get_value(key_);
    rigid_body_compositions_[rb].members.push_back(p);
    rigid_body_compositions_[rb].rb =
        core::RigidBodyMember(m, p).get_rigid_body().get_particle_index();
  }
  return true;
}


void HierarchyLoadGlobalCoordinates::fix_internal_coordinates(
    core::RigidBody rb, algebra::ReferenceFrame3D rf,
    core::RigidBodyMember rm) const {
  // Make sure the internal coordinates of the particles match
  // This is needed to handle scripts that change them during optimation
  // and save the result out to RMF.
  if (core::RigidBody::get_is_setup(rm)) {
    core::RigidBody crb(rm);
    algebra::ReferenceFrame3D crf = crb.get_reference_frame();
    algebra::ReferenceFrame3D lcrf = rf.get_local_reference_frame(crf);
    rm.set_internal_transformation(lcrf.get_transformation_to());
  } else {
    algebra::Vector3D crf = rm.get_coordinates();
    algebra::Vector3D lcrf = rf.get_local_coordinates(crf);
    rm.set_internal_coordinates(lcrf);
  }
}

void HierarchyLoadGlobalCoordinates::fix_rigid_body(Model *m,
                                                    const RB &in) const {
  IMP_LOG_TERSE("Fixing rigid body " << m->get_particle_name(in.rb)
                << std::endl);
  // core::RigidMembers rms=in.second;
  core::RigidBody rb(m, in.rb);
  kernel::ParticleIndexes rigid_bits;
  IMP_FOREACH(kernel::ParticleIndex pi, in.members) {
    if (core::RigidMember::get_is_setup(rb.get_model(), pi)) {
      rigid_bits.push_back(pi);
    }
  }
  IMP_USAGE_CHECK(!rigid_bits.empty(), "No rigid particles to align rigid"
                                           << " body with");
  rb.set_reference_frame_from_members(rigid_bits);
  algebra::ReferenceFrame3D rf = rb.get_reference_frame();
  // fix rigid bodies that aren't rigid
  IMP_FOREACH(kernel::ParticleIndex mb, in.members) {
    if (core::NonRigidMember::get_is_setup(rb.get_model(), mb)) {
      fix_internal_coordinates(rb, rf,
                               core::RigidBodyMember(rb.get_model(), mb));
    }
  }
}


void HierarchyLoadGlobalCoordinates::initialize_rigid_body(Model *m,
                                                           RB &in) const {
  IMP_LOG_TERSE("Initializing rigid body " << m->get_particle_name(in.rb)
                << std::endl);
  // core::RigidMembers rms=in.second;
  core::RigidBody rb(m, in.rb);
  algebra::ReferenceFrame3D rf =
      core::get_initial_reference_frame(m, in.members);
  IMP_LOG_TERSE("Initial rf is " << rf << std::endl);
  rb.set_reference_frame_lazy(rf);
  IMP_FOREACH(kernel::ParticleIndex pi, in.members) {
    core::RigidBodyMember rbm(m, pi);
    if (core::RigidBody::get_is_setup(m, pi)) {
      algebra::Transformation3D lc =
          rf.get_local_reference_frame(
                 core::RigidBody(m, pi).get_reference_frame())
              .get_transformation_to();
      rbm.set_internal_transformation(lc);
      IMP_LOG_VERBOSE("Internal transform for " << m->get_particle_name(pi)
                      << " are " << lc << std::endl);
    } else {
      algebra::Vector3D coords = rbm.get_coordinates();
      algebra::Vector3D lc = rf.get_local_coordinates(coords);
      IMP_LOG_VERBOSE("Internal coords for " << m->get_particle_name(pi)
                      << " are " << lc << std::endl);
      rbm.set_internal_coordinates(lc);
    }
  }
  in.initialized = true;
}

void HierarchyLoadGlobalCoordinates::load(RMF::FileConstHandle fh, Model *m) {
  IMP_FOREACH(Pair pp, xyzs_) {
    algebra::Vector3D v =
        get_coordinates(fh.get_node(pp.first), intermediate_particle_factory_);
    IMP_LOG_VERBOSE("Loading particle " << m->get_particle_name(pp.second)
                    << " to " << v << std::endl);
    core::XYZ(m, pp.second).set_coordinates(v);
  }
  IMP_FOREACH(Pair pp, rigid_bodies_) {
    algebra::ReferenceFrame3D rf(
        get_transformation(fh.get_node(pp.first), reference_frame_factory_));
    core::RigidBody(m, pp.second).set_reference_frame(rf);
  }
  typedef std::pair<const int, RB> P;
  IMP_FOREACH(P & pp, rigid_body_compositions_) {
    if (!pp.second.initialized) {
      initialize_rigid_body(m, pp.second);
    } else {
      fix_rigid_body(m, pp.second);
    }
  }
}

HierarchySaveGlobalCoordinates::HierarchySaveGlobalCoordinates(
    RMF::FileHandle f)
    : intermediate_particle_factory_(f), reference_frame_factory_(f) {
  RMF::Category cat = f.get_category("IMP");
  key_ = f.get_key<RMF::IntTraits>(cat, "rigid body");
  non_rigid_key_ = f.get_key<RMF::IntTraits>(cat, "non rigid");
}

bool HierarchySaveGlobalCoordinates::setup_node(
    kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
    kernel::ParticleIndexes rigid_bodies) {
  // assume not static
  if (!rigid_bodies.empty()) return false;
  if (core::RigidBody::get_is_setup(m, p)) {
    rigid_bodies_.push_back(std::make_pair(n.get_id(), p));
  } else if (core::XYZ::get_is_setup(m, p)) {
    xyzs_.push_back(std::make_pair(n.get_id(), p));
  }
  if (core::RigidBodyMember::get_is_setup(m, p)) {
    kernel::ParticleIndex rbpi = core::RigidBodyMember(m, p).get_rigid_body();
    if (bodies_.find(rbpi) == bodies_.end()) {
      int index = bodies_.size();
      bodies_[rbpi] = index;
    }
    n.set_value(key_, bodies_.find(rbpi)->second);
    if (core::NonRigidMember::get_is_setup(m, p)) {
      n.set_value(non_rigid_key_, 1);
    }
  }
  return true;
}

void HierarchySaveGlobalCoordinates::save(kernel::Model *m,
                                          RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, xyzs_) {
    copy_to_frame_particle(core::XYZ(m, pp.second).get_coordinates(),
                             fh.get_node(pp.first),
                             intermediate_particle_factory_);
  }
  IMP_FOREACH(Pair pp, rigid_bodies_) {
    copy_to_frame_reference_frame(core::RigidBody(m, pp.second)
                                        .get_reference_frame()
                                        .get_transformation_to(),
                                    fh.get_node(pp.first),
                                    reference_frame_factory_);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
