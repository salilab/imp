/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/utility.h>
#include <IMP/internal/ListLikeContainer.h>
#include <IMP/internal/ContainerScoreState.h>
#include <IMP/log_macros.h>
#include <IMP/utility.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

MovedSingletonContainer::MovedSingletonContainer(SingletonContainer *pc,
                                                 double threshold,
                                                 std::string name)
    : IMP::internal::ListLikeContainer<SingletonContainer>(
          pc->get_model(), name),
      threshold_(threshold),
      pc_(pc) {
  // make sure it doesn't match anything at the start
  pc_version_ = -1;
  score_state_ =
      new IMP::internal::ContainerScoreState<MovedSingletonContainer>(this);
}

void MovedSingletonContainer::do_score_state_after_evaluate() {
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE_AND_INTERNAL) { validate(); }
}

void MovedSingletonContainer::do_score_state_before_evaluate() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_CHECK_OBJECT(pc_);
  if (pc_version_ != pc_->get_contents_hash()) {
    pc_version_ = pc_->get_contents_hash();
    IMP_LOG_TERSE("First call" << std::endl);
    initialize();
  } else {
    ParticleIndexes mved = do_get_moved();
    IMP_LOG_TERSE("Setting moved list: " << Showable(mved) << std::endl);
    swap(mved);
  }
  IMP_IF_CHECK(USAGE_AND_INTERNAL) { validate(); }
}

ParticleIndexes MovedSingletonContainer::get_all_possible_indexes() const {
  return pc_->get_all_possible_indexes();
}
ParticleIndexes MovedSingletonContainer::get_range_indexes() const {
  return pc_->get_range_indexes();
}

ModelObjectsTemp MovedSingletonContainer::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret.push_back(pc_);
  ret.push_back(score_state_);
  return ret;
}

ModelObjectsTemp MovedSingletonContainer::get_score_state_inputs() const {
  ModelObjectsTemp ret =
      IMP::get_particles(get_model(), pc_->get_indexes());
  ret.push_back(pc_);
  ret += get_extra_inputs();
  return ret;
}

void MovedSingletonContainer::reset() {
  IMP_LOG_TERSE("Resetting all particles" << std::endl);
  do_reset_all();
  ParticleIndexes t;
  swap(t);
}

void MovedSingletonContainer::initialize() {
  ParticleIndexes pt = do_initialize();
  swap(pt);
}

void MovedSingletonContainer::reset_moved() {
  IMP_LOG_TERSE("Resetting moved particles" << std::endl);
  do_reset_moved();
  ParticleIndexes t;
  swap(t);
}

void MovedSingletonContainer::set_threshold(double d) {
  threshold_ = d;
  // could be more efficient, but why bother
  reset();
}

void XYZRMovedSingletonContainer::validate() const {
  IMP_OBJECT_LOG;
  ParticleIndexes pis = get_singleton_container()->get_indexes();
  IMP_USAGE_CHECK(pis.size() == backup_.size(), "Backup is not the right size");
}

void XYZRMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
  // backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(),
  { backup_.push_back(XYZR(get_model(), _1).get_sphere()); });
}
ParticleIndexes XYZRMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
  ParticleIndexes ret;
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
  ParticleIndexes ret;
  Model *m= get_model();
  IMP_CONTAINER_FOREACH(SingletonContainer, get_singleton_container(), {
    XYZR d(m, _1);
    double dr= std::abs(d.get_radius() - backup_[_2].get_radius());
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

void RigidMovedSingletonContainer::check_upper_bound(
    core::RigidBody rbs, std::pair<algebra::Sphere3D, algebra::Rotation3D> s,
    double ub) const {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  IMP_OBJECT_LOG;
  core::RigidMembers rms = rbs.get_rigid_members();
  algebra::Transformation3D tr(s.second, s.first.get_center());
  algebra::ReferenceFrame3D old(tr);
  algebra::ReferenceFrame3D cur = rbs.get_reference_frame();
  for (unsigned int i = 0; i < rms.size(); ++i) {
    algebra::Vector3D local = rms[i].get_internal_coordinates();
    algebra::Vector3D oldv = old.get_global_coordinates(local);
    algebra::Vector3D newv = cur.get_global_coordinates(local);
    double dist = get_distance(oldv, newv);
    IMP_CHECK_VARIABLE(dist);
    IMP_CHECK_VARIABLE(ub);
    IMP_INTERNAL_CHECK(dist < ub,
                       "Wrong upper bound " << ub << " on rigid body movement "
                       << " - rigid member "  << Showable(rms[i].get_particle())
                       << " old coordinates " << oldv
                       << " new coordinates " << newv
                       << " rigid body " << rbs
                       << " old rb refframe " << old
                       << " rigid body radius (as xyzr) "
                       << IMP::core::XYZR(rbs.get_particle()).get_radius()
                       << " moved by " << dist);
  }
#else
  IMP_UNUSED(rbs);
  IMP_UNUSED(s);
  IMP_UNUSED(ub);
#endif
}

void RigidMovedSingletonContainer::do_initialize_particle(
    ParticleIndex pi) {
  if (core::RigidMember::get_is_setup(get_model(), pi)) {
    core::RigidBody rb = core::RigidMember(get_model(), pi).get_rigid_body();
    ParticleIndex rbpi = rb.get_particle_index();
    if (rbs_members_.find(rbpi) == rbs_members_.end()) {
      bodies_.push_back(rbpi);
      backup_.push_back(get_data(rbpi));
    }
    rbs_members_[rbpi].push_back(pi);
  } else {
    bodies_.push_back(pi);
    rbs_members_[pi] = ParticleIndexes(1, pi);
    backup_.push_back(get_data(pi));
  }
}

ParticleIndexes RigidMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Initializing rigid moved singleton container." << std::endl);
  backup_.clear();
  rbs_members_.clear();
  bodies_.clear();
  IMP_FOREACH(ParticleIndex pi,
              get_singleton_container()->get_contents()) {
    do_initialize_particle(pi);
  }
  for (unsigned int i = 0; i < bodies_.size(); ++i) {
    moved_.insert(i);
  }
  return get_singleton_container()->get_indexes();
}

void RigidMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  moved_.clear();
  for (unsigned int i = 0; i < bodies_.size(); ++i) {
    backup_[i] = get_data(bodies_[i]);
  }
}

void RigidMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  IMP_FOREACH(int m, moved_) { backup_[m] = get_data(bodies_[m]); }
  moved_.clear();
}

// This operator is only used by IMP_INTERNAL_CHECK below, so omit its
// definition if internal checks are disabled (to avoid the compiler
// warning about an unused definition)
#if IMP_HAS_CHECKS >= IMP_INTERNAL
namespace {
std::ostream &operator<<(
    std::ostream &out, const boost::unordered_map<ParticleIndex,
                                                  ParticleIndexes> &m) {
  typedef std::pair<ParticleIndex, ParticleIndexes> P;
  out << "{";
  IMP_FOREACH(P p, m) { out << p.first << ": " << p.second << ", "; }
  out << "}";
  return out;
}
}
#endif

ParticleIndexes RigidMovedSingletonContainer::do_get_moved() {
  IMP_OBJECT_LOG;
  ParticleIndexes ret;
  IMP_LOG_TERSE("Getting moved with " << moved_.size() << std::endl);
  for (unsigned int i = 0; i < bodies_.size(); ++i) {
    if (get_distance_upper_bound(i) > get_threshold()) {
      IMP_INTERNAL_CHECK(rbs_members_.find(bodies_[i]) != rbs_members_.end(),
                         "Can't find rigid body "
                             << bodies_[i] << " bodies are " << rbs_members_);
      IMP_INTERNAL_CHECK(!rbs_members_.find(bodies_[i])->second.empty(),
                         "No members");
      ret += rbs_members_.find(bodies_[i])->second;
      moved_.insert(i);
    }
  }
  return ret;
}

double RigidMovedSingletonContainer::get_distance_upper_bound(unsigned int i)
    const {
  IMP_OBJECT_LOG;
  core::XYZR xyzr(get_model(), bodies_[i]);
  double dr= std::abs(xyzr.get_radius() - backup_[i].first.get_radius());
  double dx =
    (xyzr.get_coordinates() - backup_[i].first.get_center()).get_magnitude();
  if (!core::RigidBody::get_is_setup(get_model(), bodies_[i])) {
    return dr + dx;;
  } else {
    core::RigidBody rb(get_model(), bodies_[i]);
    algebra::Rotation3D nrot =
        rb.get_reference_frame().get_transformation_to().get_rotation();
    algebra::Rotation3D diffrot = backup_[i].second.get_inverse() * nrot;
    double angle = algebra::get_axis_and_angle(diffrot).second;

    double drot =
        std::abs(angle * xyzr.get_radius());  // over estimate, but easy
    double ub = dr + dx + drot; // upper bound
    IMP_LOG_TERSE("upperbound calc: Angle " << angle << " radius " << xyzr.get_radius()
                  << " dr " << dr << " dx " << dx
                  << " old sphere " << backup_[i].first.get_center()
                  << " new rb sphere " << xyzr.get_coordinates()
                  << " old rb rotation " << backup_[i].second
                  << " new rb rotation " << nrot
                  << " diffrot " << diffrot
                  << std::endl)
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      // make sure upper_bound of movement is not too low
      double max_error= 1.0;
      check_upper_bound(core::RigidBody(get_model(), bodies_[i]),
                     backup_[i],
                     ub+max_error);
    }
    return ub;
  }
}

RigidMovedSingletonContainer::RigidMovedSingletonContainer(
    SingletonContainer *pc, double threshold)
    : MovedSingletonContainer(pc, threshold,
                              "RigidMovedSingletonContainer%1%") {}

ModelObjectsTemp RigidMovedSingletonContainer::get_extra_inputs()
    const {
  ModelObjectsTemp ret;
  IMP_FOREACH(ParticleIndex pi,
              get_singleton_container()->get_contents()) {
    if (core::RigidMember::get_is_setup(get_model(), pi)) {
      RigidBody rb = core::RigidMember(get_model(), pi).get_rigid_body();
      ret.push_back(rb);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      // The checks in check_upper_bound() need to touch *all* members
      ret += rb.get_rigid_members();
#endif
    }
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE
