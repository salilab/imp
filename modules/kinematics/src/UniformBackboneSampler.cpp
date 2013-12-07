/**
 * \file UniformBackboneSampler.cpp
 * \brief A class for uniform sampling of backbone angles
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/UniformBackboneSampler.h>
#include <IMP/base/random.h>

IMPKINEMATICS_BEGIN_NAMESPACE

UniformBackboneSampler::UniformBackboneSampler(
    DihedralAngleRevoluteJoints joints, DOFs dofs)
    : DOFsSampler(dofs), joints_(joints) {
  IMP_USAGE_CHECK(joints_.size() == get_number_of_dofs(),
                  "number of joints should be equal to the number "
                      << "of degrees of freedom for uniformbackbonesampler");

  // init random number generators
  for (unsigned int i = 0; i < get_number_of_dofs(); i++) {
    boost::uniform_real<> u_rand_i(get_dof(i)->get_range().first,
                                   get_dof(i)->get_range().second);
    u_rand_.push_back(u_rand_i);
  }
}

DOFValues UniformBackboneSampler::do_get_sample() const {
  DOFValues v;
  v.reserve(get_number_of_dofs());
  for (unsigned int i = 0; i < get_number_of_dofs(); i++) {
    v.push_back(u_rand_[i](IMP::base::random_number_generator));
  }
  return v;
}

void UniformBackboneSampler::apply(const DOFValues& values) {
  for (unsigned int i = 0; i < joints_.size(); i++)
    joints_[i]->set_angle(values[i]);
}

IMPKINEMATICS_END_NAMESPACE
