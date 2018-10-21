/**
 * \file UniformBackboneSampler.cpp
 * \brief A class for uniform sampling of backbone angles
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/FibrilSampler.h>
#include <IMP/random.h>

IMPKINEMATICS_BEGIN_NAMESPACE

FibrilSampler::FibrilSampler(
    TransformationJoints trans_joint,
    DihedralAngleRevoluteJoints joints, DOFs dofs)
    : trans_joint_(trans_joint), dihedral_joints_(joints), DOFsSampler(dofs) {
  IMP_USAGE_CHECK(dihedral_joints_.size() == get_number_of_dofs() + -6,
                  "FibrilSampler: number of joints should be equal to the number "
                      << "of degrees of freedom minus 5");

  // TODO: Ensure that first DOF is a Transformation Joint
  // init random number generators
  for (unsigned int i = 0; i < get_number_of_dofs(); i++) {
    boost::uniform_real<> u_rand_i(get_dof(i)->get_range().first,
                                   get_dof(i)->get_range().second);
    u_rand_.push_back(u_rand_i);
  }
}

DOFValues FibrilSampler::do_get_sample() const {
  DOFValues v;
  v.reserve(get_number_of_dofs());
  //std::cerr << "FibrilSampler.do_get_sample() :: urand size = " << u_rand_.size() << std::endl;
  for (unsigned int i = 0; i < get_number_of_dofs(); i++) {
    //std::cerr << i << " " << u_rand_[i] << " " << u_rand_[i](IMP::random_number_generator) << std::endl;
    v.push_back(u_rand_[i](IMP::random_number_generator));
  }
  return v;
}

void FibrilSampler::apply(const DOFValues& values) {

  int n_dof = values.size();

  // Last 6 DOFS are for Transformation Joint
  algebra::Vector3D trans(values[n_dof-6], values[n_dof-5], values[n_dof-4]);
  algebra::Rotation3D rot = algebra::get_rotation_from_fixed_xyz(values[n_dof-3], values[n_dof-2], values[n_dof-1]);
  //std::cout << "TF joint set to " << values[n_dof-6] << " " << values[n_dof-5] << " " << values[n_dof-4] << std::endl;
  // First, apply the dihedral degrees of freedom to the chain
  for (unsigned int i = 0; i < values.size() - 6; i++) {
    dihedral_joints_[i]->set_angle(values[i]);
    //dihedral_joints_[i]->get_owner_kf()->update_all_internal_coordinates();
    //std::cout << i << " joint set to " << values[i] << " " << dihedral_joints_[i]->get_angle() << std::endl;
  }

  // Transform fibril principle component to rotation

  // Find center of mass and transform to given values.
  algebra::Transformation3D xform(rot, trans);
  trans_joint_[0]->set_transformation_child_to_parent(xform);
}

IMPKINEMATICS_END_NAMESPACE
