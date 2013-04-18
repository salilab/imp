/**
 * \file UniformBackboneSampler.cpp
 * \brief A class for uniform sampling of backbone angles
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/UniformBackboneSampler.h>

IMPKINEMATICS_BEGIN_NAMESPACE

UniformBackboneSampler::UniformBackboneSampler(
                               DihedralAngleRevoluteJoints joints,
                               DOFs dofs) :
  joints_(joints), dofs_(dofs)
{
  IMP_USAGE_CHECK(joints_.size() == dofs_.size(),
                  "number of joints should be equal to the number "
                  << "of degrees of freedom for uniformbackbonesampler");

  // init random number generators
  for(unsigned int i=0; i<dofs_.size(); i++) {
    boost::uniform_real<> u_rand_i(dofs_[i]->get_range().first,
                                   dofs_[i]->get_range().second);
    u_rand_.push_back(u_rand_i);
  }
  // save last sample as current DOFs
  last_sample_ = DOFValues(dofs_);
}

DOFValues UniformBackboneSampler::get_sample() const {
  DOFValues v;
  v.reserve(dofs_.size());
  for(unsigned int i=0; i<dofs_.size(); i++) {
    v.push_back(u_rand_[i](const_cast<UniformBackboneSampler*>(this)->rng_));
  }
  const_cast<UniformBackboneSampler*>(this)->last_sample_ = v;
  return v;
}

void UniformBackboneSampler::apply(const DOFValues& values) {
  for(unsigned int i=0; i<joints_.size(); i++)
    joints_[i]->set_angle(values[i]);
}

IMPKINEMATICS_END_NAMESPACE
