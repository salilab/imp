/**
 *  \file MovedSingletonContainer.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */


#include <IMP/core/internal/MovedSingletonContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/utility.h>
#include <IMP/SingletonModifier.h>
#include <IMP/PairModifier.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>
#include <IMP/core/internal/singleton_helpers.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE


MovedSingletonContainer::MovedSingletonContainer(SingletonContainer *pc,
                                                 double threshold):
  internal::ListLikeSingletonContainer(pc->get_model(),
                                       "MovedSingletonContainer%1%"),
  threshold_(threshold),
  pc_(pc), ac_(pc_->get_added_container()),
  rc_(pc_->get_removed_container())
{
  initialize_active_container(pc->get_model());
  first_call_=true;
}


void MovedSingletonContainer::do_show(std::ostream &) const
{
}


void MovedSingletonContainer::do_after_evaluate() {
  internal::ListLikeSingletonContainer::do_after_evaluate();
}

void MovedSingletonContainer::do_before_evaluate()
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(pc_);
  if (first_call_
      || ac_->get_number_of_particles() != 0
      || rc_->get_number_of_particles() != 0) {
    IMP_LOG(TERSE, "First call" << std::endl);
    reset();
    if (first_call_) {
      ParticlesTemp pt=pc_->get_particles();
      update_list(pt);
    }
  } else {
    ParticlesTemp mved= do_get_moved();
    add_to_list(mved);
  }
}


ParticlesTemp MovedSingletonContainer::get_state_input_particles() const {
  return pc_->get_particles();
}

ContainersTemp MovedSingletonContainer::get_state_input_containers() const {
  return ContainersTemp(1, pc_);
}

void MovedSingletonContainer::reset()
{
  do_reset_all();
  ParticlesTemp t;
  update_list(t);
  first_call_=false;
}


void MovedSingletonContainer::reset_moved()
{
  do_reset_moved();
  ParticlesTemp t;
  update_list(t);
}

void MovedSingletonContainer::set_threshold(double d) {
  threshold_= d;
  // could be more efficient, but why bother
  reset();
}

IMP_ACTIVE_CONTAINER_DEF(MovedSingletonContainer,);





void XYZRMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  //backup_.clear();
  backup_.resize(get_singleton_container()->get_number_of_particles());
  IMP_FOREACH_SINGLETON(get_singleton_container(),{
      backup_[_2]= XYZR(_1).get_sphere();
    });
}
void XYZRMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  for (unsigned int i=0; i< moved_.size(); ++i) {
    backup_[moved_[i]]
      =XYZR(get_singleton_container()->get_particle(moved_[i])).get_sphere();
  }
  moved_.clear();
}
ParticlesTemp XYZRMovedSingletonContainer::do_get_moved() {
    IMP_OBJECT_LOG;
    ParticlesTemp ret;
    IMP_FOREACH_SINGLETON(get_singleton_container(),{
        XYZR d(_1);
        double dr= std::abs(d.get_radius()- backup_[_2].get_radius());
        if (!algebra::get_interiors_intersect(
               algebra::Sphere3D(d.get_coordinates(),0),
               algebra::Sphere3D(backup_[_2].get_center(),
                                 get_threshold()-dr))) {
          ret.push_back(_1);
          moved_.push_back(_2);
        }
    });
    return ret;
  }

double XYZRMovedSingletonContainer
::do_get_distance_moved(unsigned int i) const {
  return (backup_[i].get_center()
          -core::XYZ(get_singleton_container()
                     ->get_particle(i)).get_coordinates()).get_magnitude()
    + std::abs(backup_[i].get_radius()
               - core::XYZR(get_singleton_container()
                            ->get_particle(i)).get_radius());
  }
XYZRMovedSingletonContainer
::XYZRMovedSingletonContainer(SingletonContainer *pc,
                              double threshold):
  MovedSingletonContainer(pc, threshold){
}













void RigidMovedSingletonContainer::do_reset_all() {
  IMP_OBJECT_LOG;
  ParticlesTemp normal;
  rbs_.clear();
  rbs_backup_.clear();
  rbs_members_.clear();
  IMP_FOREACH_SINGLETON(get_singleton_container(),
                        {
         if (core::RigidMember::particle_is_instance(_1)) {
           core::RigidBody rb= core::RigidMember(_1).get_rigid_body();
              if (rbs_members_.find(rb) == rbs_members_.end()) {
                rbs_.push_back(rb);
                rbs_backup_.push_back(get_data(rb));
              }
              rbs_members_[rb].push_back(_1);
         } else {
           normal.push_back(_1);
         }
                        });
  normal_->set_particles(normal);
  normal_moved_->reset();
  //backup_.clear();
  rbs_backup_.resize(get_singleton_container()->get_number_of_particles());
}
void RigidMovedSingletonContainer::do_reset_moved() {
  IMP_OBJECT_LOG;
  normal_moved_->reset_moved();
  for (unsigned int i=0; i< rbs_moved_.size(); ++i) {
    rbs_backup_[rbs_moved_[i]]=get_data(rbs_[rbs_moved_[i]]);
  }
  rbs_moved_.clear();
}


ParticlesTemp RigidMovedSingletonContainer::do_get_moved() {
    IMP_OBJECT_LOG;
    ParticlesTemp ret= normal_->get_particles();
    for (unsigned int i=0; i< rbs_.size(); ++i) {
      RigidBody rb(rbs_[i]);
      if (get_distance_estimate(rbs_[i]) > get_threshold()) {
        ret.insert(ret.end(), rbs_members_[rb].begin(), rbs_members_[rb].end());
        rbs_moved_.push_back(i);
      }
    }
    return ret;
  }

double RigidMovedSingletonContainer
::do_get_distance_moved(unsigned int i) const {
  Particle *p=get_singleton_container()->get_particle(i);
  if (core::RigidMember::particle_is_instance(p)) {
    core::RigidBody rb = core::RigidMember(p).get_rigid_body();
    return get_distance_estimate(rb);
  } else {
    return normal_moved_->get_distance_moved(p);
  }
}
RigidMovedSingletonContainer
::RigidMovedSingletonContainer(SingletonContainer *pc,
                               double threshold):
  MovedSingletonContainer(pc, threshold){
  normal_= new CoreListSingletonContainer(pc->get_model(),
                                          "Non-rigid particles %1%");
  normal_moved_= new XYZRMovedSingletonContainer(normal_, threshold);
}



ParticlesTemp RigidMovedSingletonContainer
::get_state_input_particles() const {
  ParticlesTemp ret
    = MovedSingletonContainer::get_singleton_container()
    ->get_particles();
  int sz= ret.size();
  for (int i=0; i< sz; ++i) {
    if (RigidMember::particle_is_instance(ret[i])) {
      ret.push_back(RigidMember(ret[i]).get_rigid_body());
    }
  }
  return ret;
}

ContainersTemp RigidMovedSingletonContainer
::get_state_input_containers() const {
  ContainersTemp ret;
  ret.push_back(get_singleton_container());
  ret.push_back(normal_);
  ret.push_back(normal_moved_);
  return ret;
}



IMPCORE_END_INTERNAL_NAMESPACE
