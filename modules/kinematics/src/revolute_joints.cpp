/**
 *  \file revolute_joints.cpp
 *  \brief functionality for defining various revolute kinematic
 *         joints between rigid bodies as part of a kinematic tree,
 *         including RevoluteJoint, DihedralAngleRevoluteJoint, and
 *         BondAngleRevoluteJoint
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */


#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/revolute_joints.h>
#include <IMP/core/internal/angle_helpers.h>
#include <IMP/Object.h>
//#include <IMP/compatibility/nullptr.h>
#include <IMP/exception.h>
#include <IMP/algebra/Transformation3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE


/********************** Revolute Joint ***************/

RevoluteJoint::RevoluteJoint
( IMP::core::RigidBody parent, IMP::core::RigidBody child  )
  : Joint(parent, child)
{
  //      ss=new RevoluteJointScoreState(p, ...); // TODO: implement that?
  //p->get_model()->add_score_state(ss); // TODO: implement that?
}

// definition of dummy pure virtual, just for SWIG
RevoluteJoint::~RevoluteJoint()
{}

// assumes angle and parent reference frame are updated
// and uses witnesses to get axis of rotation
// TODO: merge with Joint::update_child... and separate
//       different parts only?
void
RevoluteJoint::update_child_node_reference_frame() const
{
  using namespace IMP::algebra;
  IMP_LOG( VERBOSE,
           "Updating child node reference frame of RevoluteJoint with angle "
           << IMP_RAD_2_DEG(angle_) << " and last updated angle "
           << IMP_RAD_2_DEG(last_updated_angle_) << std::endl );

  // Preparations:
  Transformation3D R = get_rotation_about_joint_in_parent_coordinates();
  const Transformation3D& tr_child_to_parent_old =
    get_transformation_child_to_parent_no_checks();

  // Actual stuff - propagate parent transformation + rotation to child:
  Transformation3D tr_child_to_parent_new =
    R * tr_child_to_parent_old;
  last_updated_angle_ = angle_;
  const_cast<RevoluteJoint*>(this)
    ->set_transformation_child_to_parent_no_checks
    ( tr_child_to_parent_new );

  Joint::update_child_node_reference_frame();


  IMP_LOG( VERBOSE, "new child_to_parent trans " << tr_child_to_parent_new
           << std::endl);
}


double
RevoluteJoint::get_angle() const
{
  if(get_owner_kf()){
    get_owner_kf()->update_all_internal_coordinates( );
  }
  return angle_;
}


//   sets the angle of the revolute joint and update the joint
//   transformation accordingly
void
RevoluteJoint::set_angle(double angle) {
  if(get_owner_kf()){
    get_owner_kf()->update_all_internal_coordinates();
  }
  angle_ = angle;
  if(get_owner_kf()){
    get_owner_kf()->mark_internal_coordinates_changed();
  }
}

/********************** DihedralAngleRevoluteJoint ***************/

DihedralAngleRevoluteJoint
::DihedralAngleRevoluteJoint
(IMP::core::RigidBody parent, IMP::core::RigidBody child,
 IMP::core::XYZ a, IMP::core::XYZ b,
 IMP::core::XYZ c, IMP::core::XYZ d) :
  RevoluteJoint(parent, child),
  a_(a), b_(b), c_(c), d_(d) // TODO: are b_ and c_ redundant?
{
  // TODO: scorestate for udpating the model? see revolute joint
  update_axis_of_rotation_from_cartesian_witnesses();
  update_joint_from_cartesian_witnesses(); // angle only
}


double
DihedralAngleRevoluteJoint
::get_current_angle_from_cartesian_witnesses() const
{
  // TODO: add derivative support?
  return IMP::core::get_dihedral(a_, b_, c_, d_);
}



/********************** BondAngleRevoluteJoint ***************/

// control the bond angle a-b-c
BondAngleRevoluteJoint
::BondAngleRevoluteJoint
(IMP::core::RigidBody parent, IMP::core::RigidBody child,
 IMP::core::XYZ a, IMP::core::XYZ b, IMP::core::XYZ c) :
  RevoluteJoint(parent, child),
  a_(a), b_(b), c_(c)
{
  // TODO: scorestate for udpating the model? see revolute joint
  update_axis_of_rotation_from_cartesian_witnesses();
  update_joint_from_cartesian_witnesses();
}

double
BondAngleRevoluteJoint
::get_current_angle_from_cartesian_witnesses() const
{
  // TODO: add derivative support?
  return IMP::core::internal::angle(a_, b_, c_, nullptr, nullptr, nullptr);
}



IMPKINEMATICS_END_NAMESPACE
