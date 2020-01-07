/**
 * \file CCDLoopClosure.cpp
 * \brief
 *
 * \authors Dina Schneidman, Yannick Spill
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/CCDLoopClosure.h>

#include <IMP/kinematics/KinematicForest.h>

IMPKINEMATICS_BEGIN_NAMESPACE

CCDLoopClosure::CCDLoopClosure(DihedralAngleRevoluteJoints joints,
                               const algebra::Vector3D& F1,
                               const algebra::Vector3D& F2,
                               const algebra::Vector3D& F3,
                               core::XYZ M1, core::XYZ M2, core::XYZ M3) :
  joints_(joints),
  F1_(F1), F2_(F2), F3_(F3),
  M1_(M1), M2_(M2), M3_(M3) {}

bool CCDLoopClosure::close_loop() {
  double S = distance_function_S();
  const double minimized_S = 0.1; // threshold for closed loop
  if(S<minimized_S) return true; // the loop is closed

  unsigned int iteration_number = 0;
  const unsigned int max_iteration_number = 200;// seems good approximation

  while(S > minimized_S && iteration_number < max_iteration_number) {
    // iterate all joints to minimize the distance_function_S
    double curr_S = S;
    for(unsigned int i=joints_.size()-1; i>0; i--) {
      optimize_joint(i);
      curr_S = distance_function_S();
      if(curr_S < minimized_S) return true; // the loop is closed
    }

    S = curr_S;
    iteration_number++;
  }

  return false;
}


double CCDLoopClosure::distance_function_S() const {
  double res = algebra::get_squared_distance(F1_, M1_.get_coordinates())
    + algebra::get_squared_distance(F2_, M2_.get_coordinates())
    + algebra::get_squared_distance(F3_, M3_.get_coordinates());
  return std::sqrt(res);
}

void CCDLoopClosure::optimize_joint(unsigned int joint_id) {
  //get axis and origin
  algebra::Vector3D origin = joints_[joint_id]->get_b().get_coordinates();
  algebra::Vector3D axis = (joints_[joint_id]->get_c().get_coordinates()
                            - origin).get_unit_vector();

  // get current positions
  algebra::Vector3D MO1 = M1_.get_coordinates();
  algebra::Vector3D MO2 = M2_.get_coordinates();
  algebra::Vector3D MO3 = M3_.get_coordinates();

  // Calculate O1, O2, O3 as the projections of current target positions
  // on the joint axis
  algebra::Vector3D O1 = origin + ( (MO1-origin)*axis ) * axis;
  algebra::Vector3D O2 = origin + ( (MO2-origin)*axis ) * axis;
  algebra::Vector3D O3 = origin + ( (MO3-origin)*axis ) * axis;

  // Calculate vectors r1, r2, r3 (r = M-O)
  algebra::Vector3D r1 = MO1 - O1;
  algebra::Vector3D r2 = MO2 - O2;
  algebra::Vector3D r3 = MO3 - O3;

  // calculate their norms
  double mag_r1 = r1.get_magnitude();
  double mag_r2 = r2.get_magnitude();
  double mag_r3 = r3.get_magnitude();

  // norm the vectors
  algebra::Vector3D zero;
  zero[0]=0; zero[1]=0; zero[2]=0;
  algebra::Vector3D r1_unit = (mag_r1>0) ? (r1 / mag_r1) : zero;
  algebra::Vector3D r2_unit = (mag_r2>0) ? (r2 / mag_r2) : zero;
  algebra::Vector3D r3_unit = (mag_r3>0) ? (r3 / mag_r3) : zero;

  // Calculate vectors s1, s2, s3 (s = r & axis)
  algebra::Vector3D s1 = algebra::get_vector_product(axis, r1_unit);
  algebra::Vector3D s2 = algebra::get_vector_product(axis, r2_unit);
  algebra::Vector3D s3 = algebra::get_vector_product(axis, r3_unit);

  // Calculate vec_f1, vec_f2, vec_f3.
  algebra::Vector3D f1 = F1_ - O1;
  algebra::Vector3D f2 = F2_ - O2;
  algebra::Vector3D f3 = F3_ - O3;

  double numerator = (f1*s1)*mag_r1 + (f2*s2)*mag_r2 + (f3*s3)*mag_r3;
  double denominator = f1*r1 + f2*r2 + f3*r3;
  double alpha = atan2(numerator,denominator);

  joints_[joint_id]->set_angle(joints_[joint_id]->get_angle() + alpha);
  joints_[joint_id]->get_owner_kf()->update_all_external_coordinates();
}

IMPKINEMATICS_END_NAMESPACE
