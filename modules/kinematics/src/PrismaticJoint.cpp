/**
 *  \file joints.cpp
 *  \brief a prismatic joint between rigid bodies
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/core/rigid_bodies.h>
#include <IMP/kinematics/KinematicForest.h>
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/PrismaticJoint.h>
#include <IMP/Object.h>
//#include <IMP/compatibility/nullptr.h>
#include <IMP/exception.h>
#include <IMP/algebra/Transformation3D.h>

IMPKINEMATICS_BEGIN_NAMESPACE



/********************** Prismatic Joint ***************/

PrismaticJoint::PrismaticJoint
(IMP::core::RigidBody parent, IMP::core::RigidBody child,
 IMP::core::XYZ a, IMP::core::XYZ b) :
  Joint(parent, child), a_(a), b_(b)
{
  update_joint_from_cartesian_witnesses();
}

double
PrismaticJoint::get_length() const
{
  if(get_owner_kf()){
    get_owner_kf()->update_all_internal_coordinates( );
  }
  return l_;
}

void
PrismaticJoint::set_length
(double l)
{
  IMP_USAGE_CHECK( l > 0 ,
                   "Only a strictly positive length is expected for"
                   << " prismatic joints" );
  if(get_owner_kf()){
    get_owner_kf()->update_all_internal_coordinates();
  }
  l_ = l;
  IMP::algebra::Vector3D v =
    b_.get_coordinates() - a_.get_coordinates();
  IMP::algebra::Vector3D translation =
    l_ * v.get_unit_vector();
  set_transformation_child_to_parent_no_checks
    ( IMP::algebra::Transformation3D( translation ) );
  if(get_owner_kf()){
    get_owner_kf()->mark_internal_coordinates_changed();
  }
  // note: lazy so we don't update coords of b
}

void
PrismaticJoint::update_joint_from_cartesian_witnesses()
{
  using namespace IMP::algebra;
  const double tiny_double = 1e-12;
  IMP_USAGE_CHECK
    ( get_distance(a_.get_coordinates(), b_.get_coordinates())
      > tiny_double,
      "witnesses of prismatic joint should have different"
      << " coordinates" );

  Vector3D v =
    b_.get_coordinates() - a_.get_coordinates();
  double mag = v.get_magnitude();
  l_ = mag;
  // TODO: should implement set_transformation instead?
  set_transformation_child_to_parent_no_checks
    ( IMP::algebra::Transformation3D( v ) );

  IMP_UNUSED(tiny_double);
}



IMPKINEMATICS_END_NAMESPACE
