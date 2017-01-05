/**
 *  \file RevoluteJointMover.cpp
 *  \brief A mover which perturbs a joint with a gaussian.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/RevoluteJointMover.h>
#include <IMP/macros.h>
#include <IMP/random.h>
#include <IMP/warning_macros.h>
#include <IMP/core/rigid_bodies.h>
#include <boost/random/normal_distribution.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

RevoluteJointMover::RevoluteJointMover(Model *m,
        const kinematics::RevoluteJoints& joints,
        const double stddev)
    : MonteCarloMover(m, "RevoluteJointMover%1%"),
      joints_(joints), stddev_(stddev) {
  originals_.resize(joints.size(), 0.);
}

IMP_GCC_DISABLE_WARNING(-Wuninitialized)
core::MonteCarloMoverResult RevoluteJointMover::do_propose() {
  IMP_OBJECT_LOG;
  boost::normal_distribution<double> mrng(0, stddev_);
  boost::variate_generator<RandomNumberGenerator &,
                           boost::normal_distribution<double> >
      sampler(random_number_generator, mrng);

  for (unsigned int i = 0; i < joints_.size(); ++i) {
    originals_[i] = joints_[i]->get_angle();
    joints_[i]->set_angle(originals_[i] + sampler());
  }
  //get changed particles' coordinates
  ParticleIndexes idx;
  core::RigidMembers tmp(joints_[0]->get_parent_node().get_rigid_members());
  for (unsigned int i = 0; i < tmp.size(); ++i)
      idx.push_back(tmp[i]->get_index());
  for (unsigned int j = 0; j < joints_.size(); ++j) {
      tmp = joints_[j]->get_child_node().get_rigid_members();
      for (unsigned int i = 0; i < tmp.size(); ++i)
          idx.push_back(tmp[i]->get_index());
  }
  return core::MonteCarloMoverResult(idx, 1.0);
}

void RevoluteJointMover::do_reject() {
  IMP_OBJECT_LOG;
  for (unsigned int i = 0; i < joints_.size(); ++i) {
    joints_[i]->set_angle(originals_[i]);
  }
}

ModelObjectsTemp RevoluteJointMover::do_get_inputs() const {
  ModelObjectsTemp ret;
  core::RigidMembers tmp(joints_[0]->get_parent_node().get_rigid_members());
  for (unsigned int i = 0; i < tmp.size(); ++i)
      ret.push_back(tmp[i]); //->get_particle());
  for (unsigned int j = 0; j < joints_.size(); ++j) {
      tmp = joints_[j]->get_child_node().get_rigid_members();
      for (unsigned int i = 0; i < tmp.size(); ++i)
          ret.push_back(tmp[i]); //->get_particle());
  }
  return ret;
}

IMPKINEMATICS_END_NAMESPACE
