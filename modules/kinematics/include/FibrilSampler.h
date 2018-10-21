/**
 \file IMP/kinematics/FibrilSampler.h
 \brief A class for simultaneous sampling of diheral angles and fibril symmetry operations

 \authors Daniel Saltzberg
 Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_FIBRIL_SAMPLER_H
#define IMPKINEMATICS_FIBRIL_SAMPLER_H

#include "kinematics_config.h"
#include "DOFsSampler.h"
#include "revolute_joints.h"
#include "TransformationJoint.h"
#include <boost/random/uniform_real.hpp>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>
//#include <IMP/algebra/ReferenceFrame3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE

class IMPKINEMATICSEXPORT FibrilSampler : public DOFsSampler {
 public:
  /**
     Construct a sampler that samples uniformly a set of backbone (and chi)
     dihedrals as well as 
     @param trans_joint A TransformationJoint that contains the  XYZ position of the fibril root node
     		as well as the pitch/roll/yaw of the fibril principal vectors wrs: Y-axis
     @param dihedral_joints the dihedral joints over which to sample uniformly
     @param dofs corresponding list of dofs for each joint in trans_joint + dihedral_joints
                 respectively, with information about minimal /
                 maximal dof values, etc.
   */
  FibrilSampler(TransformationJoints trans_joint, DihedralAngleRevoluteJoints dihedral_joints, DOFs dofs);

  virtual void apply(const DOFValues& values);

  DihedralAngleRevoluteJoints get_dihedral_joints() { return dihedral_joints_; }

  TransformationJoints get_transformation_joint() { return trans_joint_; }

 protected:
  virtual DOFValues do_get_sample() const;

 private:
  //  boost::mt19937 rng_; // init random number generator
  mutable std::vector<boost::uniform_real<> > u_rand_;
  DihedralAngleRevoluteJoints dihedral_joints_;
  TransformationJoints trans_joint_;
};

IMP_OBJECTS(FibrilSampler, FibrilSamplers);

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_FIBRIL_SAMPLER_H */
