/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_rigid_bodies.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/rigid_bodies.h>
#include <cstdlib>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadRigidBodies::HierarchyLoadRigidBodies(RMF::FileConstHandle f)
    : reference_frame_factory_(f), ip_factory_(f) {
  RMF::Category cat = f.get_category("IMP");
  external_rigid_body_index_ =
      f.get_key(cat, "rigid body index", RMF::IntTraits());
  // backwards compat
  rb_index_key_ = f.get_key(cat, "rigid body", RMF::IntTraits());
}

void HierarchyLoadRigidBodies::setup_particle(
    RMF::NodeConstHandle n, Model *m, ParticleIndex p,
    ParticleIndexes &rigid_bodies) {
  if (n.get_has_value(external_rigid_body_index_)) {
    int i = n.get_value(external_rigid_body_index_);
    if (external_rigid_body_index_map_.find(i) ==
        external_rigid_body_index_map_.end()) {
      IMP_LOG_TERSE("Creating external rigid body: " << i << std::endl);
      std::ostringstream oss;
      oss << "RB" << i;
      ParticleIndex pi = m->add_particle(oss.str());
      core::RigidBody::setup_particle(m, pi, algebra::ReferenceFrame3D());
      if (!rigid_bodies.empty()) {
        core::RigidBody(m, rigid_bodies.back()).add_member(pi);
        if (!reference_frame_factory_.get_is_static(n)) {
          core::RigidBody(m, rigid_bodies.back())
              .set_is_rigid_member(pi, false);
        }
      }
      external_rigid_body_index_map_[i] = pi;
    }
    external_rigid_bodies_[p] = external_rigid_body_index_map_.find(i)->second;

  } else if (reference_frame_factory_.get_is(n)) {
    IMP_LOG_TERSE("Setting up " << m->get_particle_name(p) << " as rigid body."
                                << std::endl);
    core::RigidBody::setup_particle(m, p, algebra::ReferenceFrame3D());
  }

  // backwards compat
  if (n.get_has_value(rb_index_key_)) {
    int rb = n.get_value(rb_index_key_);
    if (rigid_body_compositions_[rb].rb == ParticleIndex()) {
      ParticleIndex pi = m->add_particle("RB%1%");
      rigid_body_compositions_[rb].initialized = false;
      rigid_body_compositions_[rb].rb = pi;
      core::RigidBody::setup_particle(m, pi, algebra::ReferenceFrame3D());
    }
    core::RigidBody arb(m, rigid_body_compositions_.find(rb)->second.rb);
    // set dummy coordinates (add_member() will try to use them)
    if (!core::XYZ::get_is_setup(m, p)) core::XYZ::setup_particle(m, p);
    core::XYZ(m, p).set_coordinates(algebra::Vector3D(0., 0., 0.));
    if (!ip_factory_.get_is_static(n)) {
      arb.add_non_rigid_member(p);
    } else {
      arb.add_member(p);
    }
    IMP_LOG_TERSE("Particle " << m->get_particle_name(p) << " is in rigid body "
                              << std::endl);
  }

  // link it
  link_particle(n, m, p, rigid_bodies);
}

ParticleIndex HierarchyLoadRigidBodies::find_rigid_body(
    Model *m, ParticleIndex p) {
  if (external_rigid_bodies_.find(p) == external_rigid_bodies_.end()) {
    // just linking, so search for it
    atom::Hierarchy cur(m, p);
    while (!core::RigidBodyMember::get_is_setup(cur)) {
      cur = cur.get_child(0);
    }
    external_rigid_bodies_[p] =
        core::RigidBodyMember(cur).get_rigid_body().get_particle_index();
  }

  return external_rigid_bodies_.find(p)->second;
}

void HierarchyLoadRigidBodies::link_rigid_body(
    RMF::NodeConstHandle n, Model *m, ParticleIndex p,
    ParticleIndexes &rigid_bodies) {
  // Cannot use reference_frame_factory_ here because for nonrigid nested
  // rigid body members no static values are set in RMF
  if (core::RigidMember::get_is_setup(m, p)) {
    IMP_LOG_VERBOSE("Rigid body " << m->get_particle_name(p) << " is static"
                                  << std::endl);
  } else if (rigid_bodies.empty()) {
    global_.push_back(std::make_pair(n.get_id(), p));
  } else {
    local_.push_back(std::make_pair(n.get_id(), p));
  }
  rigid_bodies.push_back(p);
}

void HierarchyLoadRigidBodies::link_particle(
    RMF::NodeConstHandle n, Model *m, ParticleIndex p,
    ParticleIndexes &rigid_bodies) {
  if (n.get_has_value(external_rigid_body_index_)) {
    ParticleIndex rb = find_rigid_body(m, p);
    link_rigid_body(n, m, rb, rigid_bodies);
  } else {
    if (core::RigidBody::get_is_setup(m, p)
        || reference_frame_factory_.get_is(n)) {
      link_rigid_body(n, m, p, rigid_bodies);
    }

    // backwards compat
    if (n.get_has_value(rb_index_key_)) {
      int rb = n.get_value(rb_index_key_);
      rigid_body_compositions_[rb].members.push_back(p);
      rigid_body_compositions_[rb].rb =
          core::RigidBodyMember(m, p).get_rigid_body().get_particle_index();
      rigid_bodies.push_back(rigid_body_compositions_[rb].rb);
    }
  }
}

void HierarchyLoadRigidBodies::fix_internal_coordinates(
    core::RigidBody, algebra::ReferenceFrame3D rf,
    core::RigidBodyMember rm) const {
  // Make sure the internal coordinates of the particles match
  // This is needed to handle scripts that change them during optimization
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

void HierarchyLoadRigidBodies::fix_rigid_body(Model *m, const RB &in) const {
  IMP_LOG_TERSE("Fixing rigid body " << m->get_particle_name(in.rb)
                                     << std::endl);
  // core::RigidMembers rms=in.second;
  core::RigidBody rb(m, in.rb);
  ParticleIndexes rigid_bits;
  IMP_FOREACH(ParticleIndex pi, in.members) {
    if (core::RigidMember::get_is_setup(rb.get_model(), pi)) {
      rigid_bits.push_back(pi);
    }
  }
  if (rigid_bits.empty()) {
    // all members are non-rigid, so nothing to do
    return;
  }

  rb.set_reference_frame_from_members(rigid_bits);
  algebra::ReferenceFrame3D rf = rb.get_reference_frame();
  // fix rigid bodies that aren't rigid
  IMP_FOREACH(ParticleIndex mb, in.members) {
    if (core::NonRigidMember::get_is_setup(rb.get_model(), mb)) {
      fix_internal_coordinates(rb, rf,
                               core::RigidBodyMember(rb.get_model(), mb));
    }
  }
}

void HierarchyLoadRigidBodies::initialize_rigid_body(Model *m, RB &in) const {
  IMP_LOG_TERSE("Initializing rigid body " << m->get_particle_name(in.rb)
                                           << std::endl);
  // core::RigidMembers rms=in.second;
  core::RigidBody rb(m, in.rb);
  algebra::ReferenceFrame3D rf =
      core::get_initial_reference_frame(m, in.members);
  IMP_LOG_TERSE("Initial rf is " << rf << std::endl);
  rb.set_reference_frame_lazy(rf);
  IMP_FOREACH(ParticleIndex pi, in.members) {
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

void HierarchyLoadRigidBodies::load(RMF::FileConstHandle fh, Model *m) {
  IMP_FOREACH(Pair pp, global_) {
    IMP_LOG_VERBOSE("Loading global rigid body "
                    << m->get_particle_name(pp.second) << std::endl);
    algebra::ReferenceFrame3D rf(
        get_transformation(fh.get_node(pp.first), reference_frame_factory_));
    core::RigidBody(m, pp.second).set_reference_frame_lazy(rf);
  }
  IMP_FOREACH(Pair pp, local_) {
    IMP_LOG_VERBOSE("Loading local rigid body "
                    << m->get_particle_name(pp.second) << std::endl);
    algebra::ReferenceFrame3D rf(
        get_transformation(fh.get_node(pp.first), reference_frame_factory_));
    core::RigidBodyMember(m, pp.second)
        .set_internal_transformation(rf.get_transformation_to());
  }
}

void HierarchyLoadRigidBodies::update_rigid_bodies(RMF::FileConstHandle,
                                                   Model *m) {
  /* Make sure that the global coordinates of any nested rigid bodies are
     set from their parents */
  IMP_FOREACH(Pair pp, global_) {
    core::RigidBody rb(m, pp.second);
    rb.update_members();
  }

  // backwards compat
  typedef std::pair<const int, RB> P;
  IMP_FOREACH(P & pp, rigid_body_compositions_) {
    if (!pp.second.initialized) {
      initialize_rigid_body(m, pp.second);
    } else {
      fix_rigid_body(m, pp.second);
    }
  }
}

HierarchySaveRigidBodies::HierarchySaveRigidBodies(RMF::FileHandle f)
    : reference_frame_factory_(f) {
  RMF::Category cat = f.get_category("IMP");
  external_rigid_body_index_ =
      f.get_key(cat, "rigid body index", RMF::IntTraits());
}

ParticleIndex HierarchySaveRigidBodies::fill_external(
    Model *m, ParticleIndex p) {
  RMF_SMALL_UNORDERED_SET<ParticleIndex> rbs;
  IMP_FOREACH(ParticleIndex ch,
              atom::Hierarchy(m, p).get_children_indexes()) {
    ParticleIndex cur = fill_external(m, ch);
    rbs.insert(cur);
  }
  ParticleIndex ret = IMP::get_invalid_index<ParticleIndexTag>();
  if (core::RigidBodyMember::get_is_setup(m, p)) {
    ret = core::RigidBodyMember(m, p).get_rigid_body().get_particle_index();
    rbs.insert(ret);
  }
  // If we are a rigid body, ignore children that are *not* in the rigid
  // body (they will be coerced to nonrigid members elsewhere)
  if (core::RigidBody::get_is_setup(m, p)) {
    rbs.erase(IMP::get_invalid_index<ParticleIndexTag>());
  }
  if (rbs.size() == 1 &&
      *rbs.begin() != IMP::get_invalid_index<ParticleIndexTag>()) {
    externals_[p] = *rbs.begin();
    external_index_[externals_[p]] = externals_[p].get_index();
    IMP_FOREACH(ParticleIndex ch,
                atom::Hierarchy(m, p).get_children_indexes()) {
      not_externals_.insert(ch);
      externals_.erase(ch);
    }
  } else {
    not_externals_.insert(p);
  }
  return ret;
}

void HierarchySaveRigidBodies::handle_rigid_body(
    Model *m, ParticleIndex p, RMF::NodeHandle n,
    ParticleIndexes &rigid_bodies) {
  core::RigidBody rb(m, p);
  bool nested =
      !rigid_bodies.empty() && core::RigidBodyMember::get_is_setup(m, p) &&
      core::RigidBodyMember(m, p).get_rigid_body().get_particle_index() ==
          rigid_bodies.back();
  if (nested && core::RigidMember::get_is_setup(m, p)) {
    // static
    copy_to_static_reference_frame(
        core::RigidMember(m, p).get_internal_transformation(), n,
        reference_frame_factory_);
  } else if (nested) {
    local_.push_back(std::make_pair(n.get_id(), p));
  } else {
    global_.push_back(std::make_pair(n.get_id(), p));
  }
  rigid_bodies.push_back(p);
}

void HierarchySaveRigidBodies::setup_node(
    Model *m, ParticleIndex p, RMF::NodeHandle n,
    ParticleIndexes &rigid_bodies) {
  if (externals_.find(p) == externals_.end() &&
      not_externals_.find(p) == not_externals_.end()) {
    fill_external(m, p);
  }

  if (externals_.find(p) != externals_.end()) {
    ParticleIndex rbi = externals_.find(p)->second;
    n.set_static_value(external_rigid_body_index_,
                       external_index_.find(rbi)->second);
    handle_rigid_body(m, rbi, n, rigid_bodies);
  } else if (core::RigidBody::get_is_setup(m, p)) {
    handle_rigid_body(m, p, n, rigid_bodies);
  }
}

void HierarchySaveRigidBodies::save(Model *m, RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, global_) {
    copy_to_frame_reference_frame(core::RigidBody(m, pp.second)
                                      .get_reference_frame()
                                      .get_transformation_to(),
                                  fh.get_node(pp.first),
                                  reference_frame_factory_);
  }
  IMP_FOREACH(Pair pp, local_) {
    copy_to_frame_reference_frame(
        core::RigidBodyMember(m, pp.second).get_internal_transformation(),
        fh.get_node(pp.first), reference_frame_factory_);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
