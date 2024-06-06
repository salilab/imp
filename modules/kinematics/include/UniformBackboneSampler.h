/**
 \file IMP/kinematics/UniformBackboneSampler.h
 \brief A class for uniform sampling of backbone angles

 \authors Dina Schneidman, Barak Raveh
 Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H
#define IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H

#include <IMP/kinematics/kinematics_config.h>
#include "DOFsSampler.h"
#include "revolute_joints.h"
#include <boost/random/uniform_real_distribution.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

// TODO: is it necessarily backbone? could be any dihedrals too?

//! Sample uniformly over a set of backbone dihedral joints
class IMPKINEMATICSEXPORT UniformBackboneSampler : public DOFsSampler {
 public:
  /**
     Construct a uniform backbone sampler over joints, with
     corresponding dof properties specified in dofs

     @param joints the dihedral joints over which to sample uniformly
     @param dofs corresponding list of dofs for each joint in joints
                 respectively, with information about minimal /
                 maximal dof values, etc.
   */
  UniformBackboneSampler(DihedralAngleRevoluteJoints joints, DOFs dofs);

  virtual void apply(const DOFValues& values) override;
  virtual void apply_floats(const Floats& values);

  DihedralAngleRevoluteJoints get_joints() { return joints_; }

 protected:
  virtual DOFValues do_get_sample() const override;

 private:
  //  boost::mt19937 rng_; // init random number generator
  mutable std::vector<boost::random::uniform_real_distribution<> > u_rand_;
  DihedralAngleRevoluteJoints joints_;
};

IMP_OBJECTS(UniformBackboneSampler, UniformBackboneSamplers);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H */
