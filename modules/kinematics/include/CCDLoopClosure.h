/**
 * \file IMP/kinematics/CCDLoopClosure.h
 * \brief
 *
 * \authors Dina Schneidman, Yannick Spill
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_CCD_LOOP_CLOSURE_H
#define IMPKINEMATICS_CCD_LOOP_CLOSURE_H

#include "kinematics_config.h"
#include <IMP/kinematics/revolute_joints.h>
#include <IMP/algebra/Vector3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE

//! CCD loop closure
/** see Canutescu and Dunbrack, Protein Science
 */
class IMPKINEMATICSEXPORT CCDLoopClosure {
public:
  //! Constructor.
  /** F1-3 are the target points for closed loop
      M1-3 are the points in the model
   */
  CCDLoopClosure(DihedralAngleRevoluteJoints joints,
                 const algebra::Vector3D& F1,
                 const algebra::Vector3D& F2,
                 const algebra::Vector3D& F3,
                 core::XYZ M1, core::XYZ M2, core::XYZ M3);

  bool close_loop();

 private:
  // computes the distance between target and mobile loop anchors
  double distance_function_S() const;

  void optimize_joint(unsigned int joint_id);

 private:
  DihedralAngleRevoluteJoints joints_;
  algebra::Vector3D F1_, F2_, F3_; // target loop anchors
  core::XYZ M1_, M2_, M3_; // mobile version of loop anchors

};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_CCD_LOOP_CLOSURE_H */
