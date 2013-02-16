/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */


#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/utility.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/internal/InternalListSingletonContainer.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>
#include <IMP/base/utility.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE


MovedSingletonContainer::MovedSingletonContainer(SingletonContainer *pc,
                                                 double threshold):
  IMP::internal::ListLikeSingletonContainer(pc->get_model(),
                                       "MovedSingletonContainer%1%"),
  threshold_(threshold),
  pc_(pc)
{
  first_call_=true;
  reset_all_=false;
  reset_moved_=false;
}


void MovedSingletonContainer::do_show(std::ostream &) const
{
}


void MovedSingletonContainer::do_after_evaluate(DerivativeAccumulator*da) {
  IMP::internal::ListLikeSingletonContainer::do_after_evaluate(da);
  if (reset_all_) {
    do_reset_all();
    ParticleIndexes t;
    swap(t);
  } else if (reset_moved_) {
    do_reset_moved();
    ParticleIndexes t;
    swap(t);
  }
  reset_moved_=false;
  reset_all_=false;
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    validate();
  }
}

void MovedSingletonContainer::do_before_evaluate()
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(pc_);
  if (first_call_ || pc_->get_is_changed()) {
    IMP_LOG_TERSE( "First call" << std::endl);
    initialize();
    first_call_=false;
  } else {
    ParticleIndexes mved= do_get_moved();
    IMP_LOG_TERSE( "Adding to moved list: " << Showable(mved) << std::endl);
    ParticleIndexes old;
    swap(old);
    old+=mved;
    swap(old);
  }
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    validate();
  }
}

ParticleIndexes MovedSingletonContainer::get_all_possible_indexes() const {
  return pc_->get_all_possible_indexes();
}
ParticleIndexes MovedSingletonContainer::get_range_indexes() const {
  return pc_->get_range_indexes();
}

ParticlesTemp MovedSingletonContainer::get_input_particles() const {
  return IMP::get_particles(get_model(), pc_->get_indexes());
}

ContainersTemp MovedSingletonContainer::get_input_containers() const {
  return ContainersTemp(1, pc_);
}

void MovedSingletonContainer::reset()
{
  reset_all_=true;
}

void MovedSingletonContainer::initialize()
{
  ParticleIndexes pt=do_initialize();
  swap(pt);
}


void MovedSingletonContainer::reset_moved()
{
  reset_moved_=true;
}

void MovedSingletonContainer::set_threshold(double d) {
  threshold_= d;
  // could be more efficient, but why bother
  reset();
}



void XYZRMovedSingletonContainer::validate() const {
  IMP_OBJECT_LOG;
  ParticleIndexes pis= get_singleton_container()->get_indexes();
  IMP_USAGE_CHECK(pis.size()==backup_.size(),
                  "Backup is not the right size");
}

void XYZRMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
  //backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_FOREACH_SINGLETON(get_singleton_container(),{
      backup_.push_back(XYZR(_1).get_sphere());
    });
}
ParticleIndexes XYZRMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  backup_.clear();
  moved_.clear();
  ParticleIndexes ret;
  //backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_FOREACH_SINGLETON_INDEX(get_singleton_container(),{
      backup_.push_back(XYZR(get_model(), _1).get_sphere());
      moved_.push_back(_2);
      ret.push_back(_1);
    });
  return ret;
}

void XYZRMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  std::sort(moved_.begin(), moved_.end());
  IMP_CONTAINER_ACCESS(SingletonContainer, get_singleton_container(),
                       {
  for (unsigned int i=0; i< moved_.size(); ++i) {
    // skip duplicates
    if (i > 0 && moved_[i-1]== moved_[i]) continue;
    backup_[moved_[i]]
      =XYZR(get_model(), imp_indexes[moved_[i]]).get_sphere();
  }
                       });
  moved_.clear();
}
ParticleIndexes XYZRMovedSingletonContainer::do_get_moved() {
    IMP_OBJECT_LOG;
    ParticleIndexes ret;
    Model *m= get_model();
    IMP_FOREACH_SINGLETON_INDEX(get_singleton_container(),{
        XYZR d(m, _1);
        double dr= std::abs(d.get_radius()- backup_[_2].get_radius());
        if (!algebra::get_interiors_intersect(
               algebra::Sphere3D(d.get_coordinates(),0),
               algebra::Sphere3D(backup_[_2].get_center(),
                                 std::max<double>(0,get_threshold()-dr)))) {
          ret.push_back(_1);
          moved_.push_back(_2);
        }
    });
    return ret;
  }

XYZRMovedSingletonContainer
::XYZRMovedSingletonContainer(SingletonContainer *pc,
                              double threshold):
  MovedSingletonContainer(pc, threshold){
}






void RigidMovedSingletonContainer::validate() const {
  IMP_OBJECT_LOG;
}

void RigidMovedSingletonContainer::check_estimate(core::RigidBody rbs,
                      std::pair<algebra::Sphere3D, algebra::Rotation3D> s,
                                                  double d) const {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  core::RigidMembers rms= rbs.get_members();
  algebra::Transformation3D tr(s.second,
                               s.first.get_center());
  algebra::ReferenceFrame3D old(tr);
  algebra::ReferenceFrame3D cur= rbs.get_reference_frame();
  for (unsigned int i=0; i< rms.size(); ++i) {
    algebra::Vector3D local= rms[i].get_internal_coordinates();
    algebra::Vector3D oldv= old.get_global_coordinates(local);
    algebra::Vector3D newv= cur.get_global_coordinates(local);
    double dist= get_distance(oldv, newv);
    IMP_CHECK_VARIABLE(dist);
    IMP_CHECK_VARIABLE(d);
    IMP_INTERNAL_CHECK(dist  < d+1,
                       "Particle moved further than expected "
                       << dist << " > " << d
                       << " for " << Showable(rms[i].get_particle()));
  }
#else
  IMP_UNUSED(rbs);
  IMP_UNUSED(s);
  IMP_UNUSED(d);
#endif
}


void
RigidMovedSingletonContainer::do_initialize_particle(ParticleIndex pi) {
  if (core::RigidMember::particle_is_instance(get_model(),
                                              pi)) {
    core::RigidBody rb= core::RigidMember(get_model(), pi).get_rigid_body();
    ParticleIndex rbpi=rb.get_particle_index();
    if (rbs_members_.find(rbpi) == rbs_members_.end()) {
      bodies_.push_back(pi);
      backup_.push_back(get_data(pi));
    }
    rbs_members_[rb.get_particle_index()].push_back(pi);
  } else {
    bodies_.push_back(pi);
    rbs_members_[pi]=ParticleIndexes(1, pi);
    backup_.push_back(get_data(pi));
  }
}

ParticleIndexes RigidMovedSingletonContainer::do_initialize() {
  IMP_OBJECT_LOG;
  ParticleIndexes normal;
  backup_.clear();
  rbs_members_.clear();
  bodies_.clear();
  IMP_CONTAINER_FOREACH(SingletonContainer,
                        get_singleton_container(),
                        {
                          do_initialize_particle(_1);
                        });
  return get_singleton_container()->get_indexes();
}

void RigidMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  do_initialize();
}
void RigidMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< moved_.size(); ++i) {
    backup_[moved_[i]]=get_data(bodies_[moved_[i]]);
  }
  moved_.clear();
}


ParticleIndexes RigidMovedSingletonContainer::do_get_moved() {
    IMP_OBJECT_LOG;
    ParticleIndexes ret;
    for (unsigned int i=0; i< bodies_.size(); ++i) {
      if (get_distance_estimate(i) > get_threshold()) {
        ret+= rbs_members_[bodies_[i]];
        moved_.push_back(i);
      }
    }
    return ret;
  }

RigidMovedSingletonContainer
::RigidMovedSingletonContainer(SingletonContainer *pc,
                               double threshold):
  MovedSingletonContainer(pc, threshold){
}

ParticlesTemp RigidMovedSingletonContainer
::get_input_particles() const {
  ParticlesTemp ret
    = IMP::get_particles(get_model(),
MovedSingletonContainer::get_singleton_container()
                         ->get_indexes());
  int sz= ret.size();
  for (int i=0; i< sz; ++i) {
    if (RigidMember::particle_is_instance(ret[i])) {
      ret.push_back(RigidMember(ret[i]).get_rigid_body());
    }
  }
  return ret;
}


IMPCORE_END_INTERNAL_NAMESPACE
