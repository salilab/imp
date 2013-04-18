/**
 * \file UniformBackboneSampler.h
 * \brief A class for uniform sampling of backbone angles
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H
#define IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H

#include "kinematics_config.h"
#include "DOFsSampler.h"
#include "revolute_joints.h"

IMPKINEMATICS_BEGIN_NAMESPACE

class IMPKINEMATICSEXPORT UniformBackboneSampler : public DOFsSampler {
 public:
  UniformBackboneSampler(DihedralAngleRevoluteJoints joints, DOFs dofs);

  virtual DOFValues get_sample() const;

  virtual void apply(const DOFValues& values);

 private:
  boost::mt19937 rng_; // init random number generator
  std::vector<boost::uniform_real<> > u_rand_;
  DihedralAngleRevoluteJoints joints_;
  DOFs dofs_;
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_UNIFORM_BACKBONE_SAMPLER_H */
