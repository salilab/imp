/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/kernel/internal/InternalListSingletonContainer.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>
#include <IMP/base/utility.h>


#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

MovedSingletonContainer::MovedSingletonContainer(SingletonContainer *pc,
                                                 double threshold,
                                                 std::string name)
    : IMP::internal::ListLikeSingletonContainer(pc->get_model(), name),
      threshold_(threshold),
      pc_(pc) {
  // make sure it doesn't match anything at the start
  pc_version_ = -1;
  reset_all_ = false;
  reset_moved_ = false;
}

void MovedSingletonContainer::do_after_evaluate(DerivativeAccumulator *da) {
  IMP::internal::ListLikeSingletonContainer::do_after_evaluate(da);
  if (reset_all_) {
    do_reset_all();
    kernel::ParticleIndexes t;
    swap(t);
  } else if (reset_moved_) {
    do_reset_moved();
    kernel::ParticleIndexes t;
    swap(t);
  }
  reset_moved_ = false;
  reset_all_ = false;
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) { validate(); }
}

void MovedSingletonContainer::do_before_evaluate() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(pc_);
  if (update_version(pc_, pc_version_)) {
    IMP_LOG_TERSE("First call" << std::endl);
    initialize();
  } else {
    kernel::ParticleIndexes mved = do_get_moved();
    IMP_LOG_TERSE("Adding to moved list: " << Showable(mved) << std::endl);
    kernel::ParticleIndexes old;
    swap(old);
    old += mved;
    swap(old);
  }
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) { validate(); }
}

ParticleIndexes MovedSingletonContainer::get_all_possible_indexes() const {
  return pc_->get_all_possible_indexes();
}
ParticleIndexes MovedSingletonContainer::get_range_indexes() const {
  return pc_->get_range_indexes();
}

ModelObjectsTemp MovedSingletonContainer::do_get_inputs() const {
  kernel::ModelObjectsTemp ret =
      IMP::get_particles(get_model(), pc_->get_indexes());
  ret.push_back(pc_);
  return ret;
}

void MovedSingletonContainer::reset() { reset_all_ = true; }

void MovedSingletonContainer::initialize() {
  kernel::ParticleIndexes pt = do_initialize();
  swap(pt);
}

void MovedSingletonContainer::reset_moved() { reset_moved_ = true; }

void MovedSingletonContainer::set_threshold(double d) {
  threshold_ = d;
  // could be more efficient, but why bother
  reset();
}

void XYZRMovedSingletonContainer::validate() const {
  IMP_OBJECT_LOG;
  kernel::ParticleIndexes pis = get_singleton_container()->get_indexes();
  IMP_USAGE_CHECK(pis.size() == backup_.size(), "Backup is not the right size");
}

void XYZRMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
    // backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(), {
    backup_.push_back(XYZR(get_model(), _1).get_sphere());
  });
}
ParticleIndexes XYZRMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
  kernel::ParticleIndexes ret;
    // backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(), {
    backup_.push_back(XYZR(get_model(), _1).get_sphere());
    moved_.insert(_2);
    ret.push_back(_1);
  });
  return ret;
}

void XYZRMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Resetting moved particles" << std::endl);
  IMP_CONTAINER_ACCESS(SingletonContainer, get_singleton_container(), {
      IMP_FOREACH(int m, moved_) {
        backup_[m] =
            XYZR(get_model(), imp_indexes[m]).get_sphere();
      }
  });
  moved_.clear();
}
ParticleIndexes XYZRMovedSingletonContainer::do_get_moved() {
  IMP_OBJECT_LOG;
  kernel::ParticleIndexes ret;
  kernel::Model *m = get_model();
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(), {
    if (moved_.find(_2) != moved_.end())
      continue;
    XYZR d(m, _1);
    double dr = std::abs(d.get_radius() - backup_[_2].get_radius());
    if (!algebra::get_interiors_intersect(
             algebra::Sphere3D(d.get_coordinates(), 0),
             algebra::Sphere3D(backup_[_2].get_center(),
                               std::max<double>(0, get_threshold() - dr)))) {
      ret.push_back(_1);
      moved_.insert(_2);
    }
  });
  return ret;
}

XYZRMovedSingletonContainer::XYZRMovedSingletonContainer(SingletonContainer *pc,
                                                         double threshold)
    : MovedSingletonContainer(pc, threshold, "XYZRMovedSingletonContainer%1%") {
}

void RigidMovedSingletonContainer::validate() const { IMP_OBJECT_LOG; }

void RigidMovedSingletonContainer::check_estimate(
    core::RigidBody rbs, std::pair<algebra::Sphere3D, algebra::Rotation3D> s,
    double d) const {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  core::RigidMembers rms = rbs.get_members();
  algebra::Transformation3D tr(s.second, s.first.get_center());
  algebra::ReferenceFrame3D old(tr);
  algebra::ReferenceFrame3D cur = rbs.get_reference_frame();
  for (unsigned int i = 0; i < rms.size(); ++i) {
    algebra::Vector3D local = rms[i].get_internal_coordinates();
    algebra::Vector3D oldv = old.get_global_coordinates(local);
    algebra::Vector3D newv = cur.get_global_coordinates(local);
    double dist = get_distance(oldv, newv);
    IMP_CHECK_VARIABLE(dist);
    IMP_CHECK_VARIABLE(d);
    IMP_INTERNAL_CHECK(dist < d + 1, "Particle moved further than expected "
                                         << dist << " > " << d << " for "
                                         << Showable(rms[i].get_particle()));
  }
#else
  IMP_UNUSED(rbs);
  IMP_UNUSED(s);
  IMP_UNUSED(d);
#endif
}

void RigidMovedSingletonContainer::do_initialize_particle(
    kernel::ParticleIndex pi) {
  if (core::RigidMember::get_is_setup(get_model(), pi)) {
    core::RigidBody rb = core::RigidMember(get_model(), pi).get_rigid_body();
    kernel::ParticleIndex rbpi = rb.get_particle_index();
    if (rbs_members_.find(rbpi) == rbs_members_.end()) {
      bodies_.push_back(pi);
      backup_.push_back(get_data(pi));
    }
    rbs_members_[rb.get_particle_index()].push_back(pi);
  } else {
    bodies_.push_back(pi);
    rbs_members_[pi] = kernel::ParticleIndexes(1, pi);
    backup_.push_back(get_data(pi));
  }
}

ParticleIndexes RigidMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  kernel::ParticleIndexes normal;
  backup_.clear();
  rbs_members_.clear();
  bodies_.clear();
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(), {
    do_initialize_particle(_1);
    moved_.insert(_2);
  });

  return get_singleton_container()->get_indexes();
}

void RigidMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  do_initialize();
}
void RigidMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  IMP_FOREACH(int m, moved_) { backup_[m] = get_data(bodies_[m]); }
  moved_.clear();
}

ParticleIndexes RigidMovedSingletonContainer::do_get_moved() {
  IMP_OBJECT_LOG;
  kernel::ParticleIndexes ret;
  IMP_LOG_TERSE("Getting moved with " << moved_.size() << std::endl);
  for (unsigned int i = 0; i < bodies_.size(); ++i) {
    if (moved_.find(i) == moved_.end() &&
        get_distance_estimate(i) > get_threshold()) {
      ret += rbs_members_[bodies_[i]];
      moved_.insert(i);
    }
  }
  return ret;
}

RigidMovedSingletonContainer::RigidMovedSingletonContainer(
    SingletonContainer *pc, double threshold)
    : MovedSingletonContainer(pc, threshold,
                              "RigidMovedSingletonContainer%1%") {}

ParticlesTemp RigidMovedSingletonContainer::get_input_particles() const {
  kernel::ParticlesTemp ret = IMP::get_particles(
      get_model(),
      MovedSingletonContainer::get_singleton_container()->get_indexes());
  int sz = ret.size();
  for (int i = 0; i < sz; ++i) {
    if (RigidMember::get_is_setup(ret[i])) {
      ret.push_back(RigidMember(ret[i]).get_rigid_body());
    }
  }
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE
