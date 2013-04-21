/**
 *  \file revolute_joints.h
 *  \brief functionality for defining various revolute kinematic
 *         joints between rigid bodies as part of a kinematic tree,
 *         including RevoluteJoint, DihedralAngleRevoluteJoint, and
 *         BondAngleRevoluteJoint
 *  \authors Dina Schneidman, Barak Raveh
 *

 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKINEMATICS_REVOLUTE_JOINTS_H
#define IMPKINEMATICS_REVOLUTE_JOINTS_H

#include "kinematics_config.h"
#include <IMP/kinematics/KinematicNode.h>
#include <IMP/kinematics/Joint.h>
#include <IMP/base/Object.h>
//#include <IMP/compatibility/nullptr.h>
#include <IMP/exception.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/base/check_macros.h>

// TODO: for debug only = remove later
#define IMP_RAD_2_DEG(a) 180*a/IMP::algebra::PI
#define IMP_DEG_2_RAD(a) a*IMP::algebra::PI/180

IMPKINEMATICS_BEGIN_NAMESPACE

class KinematicForest;

// inline void nice_print_trans(const IMP::algebra::Transformation3D& T,
//                              std::string description)
// {
//   std::pair< IMP::algebra::Vector3D, double > aa;
//   aa = IMP::algebra::get_axis_and_angle( T.get_rotation() );
//   IMP_LOG( VERBOSE, description << "axis = " << aa.first
//            << "; angle = " << IMP_RAD_2_DEG(aa.second) << " deg"
//            << "; translation = " << T.get_translation()
//            << std::endl );
// }


/********************** RevoluteJoint ***************/


/** Abstract class for all revolute joints **/
class IMPKINEMATICSEXPORT RevoluteJoint :
public Joint{
 public:
  /**
     constructs a revolute joint on the line connecting a and b,
     with an initial angle 'angle'

     @param parent,child kinematic nodes upstream and downstream (resp.) of this
                    joint
  **/
 RevoluteJoint(IMP::core::RigidBody parent,
               IMP::core::RigidBody child);


  // pure virtual dtr to declare as abstrat class for SWIG
 virtual ~RevoluteJoint() = 0;


 /******* getter / setter methods *********/
 public:

  /**
     sets the angle of the revolute joint and marks the internal
     coordinates as changed in the kinematic forest object
   */
  void set_angle(double angle);

  /**
     gets the angle of the revolute joint. This method is kinematically
     safe (it triggers an update to internal coordinates if needed)
  */
  double get_angle() const;

 protected:
  //#ifndef SWIG
  // in global coordinates
  const IMP::algebra::Vector3D& get_rot_axis_origin() const
    { return rot_axis_origin_; }

  // in global coordinates
  const IMP::algebra::Vector3D& get_rot_axis_unit_vector() const
    { return rot_axis_unit_vector_; }
  //#endif

  /****************** general protected methods ***************/

 protected:
  /**
     Updates the reference frame of the child node by this joint
     angle, assuming the parent reference frame and the witnesses
     that affect update_axis_of_rotation_from_cartesian_witnesses()
     are all updated already
  */
  virtual void update_child_node_reference_frame() const;

  /**
      this protected method updates the rot_axis_unit_vector_
      and rot_axis_origin_ variables based on the cartesian witnesses
      appropriate for a specific implementation of this abstract class,
      using parent coordinates, assuming all caresian witnesses are updated
  */
  virtual void update_axis_of_rotation_from_cartesian_witnesses() = 0;

  /**
      this protected method uses the cartesian witnesses to compute
      the actual current angle of this joint (assuming external coordinates
      of required cartesian witnesses are up to date).
      @note this method does not update the angle stored in this joint,
            which may be strictly different (if external or internal coords
            are outdated)
  */
  virtual double get_current_angle_from_cartesian_witnesses() const = 0;


  /**
     Update the joint internal parameters based on external reference frames
     of witnesses and rigid bodies, assuming external parameters are updated
   */
  virtual void update_joint_from_cartesian_witnesses(){
    update_axis_of_rotation_from_cartesian_witnesses();
    angle_ = get_current_angle_from_cartesian_witnesses();
    last_updated_angle_ = angle_;
    Joint::update_joint_from_cartesian_witnesses();
  }

  /**
     Returns the transformation matrix for rotating a vector in
     parent coordinates about the axis of the joint, in a way that
     would bring the cartesian witnesses to the correct joint angle
     (as measured by get_angle_from_cartesian_witnesses() ).
   */
  IMP::algebra::Transformation3D
    get_rotation_about_joint_in_parent_coordinates() const
    {
      IMP_LOG( VERBOSE, "get_rotation " << IMP_RAD_2_DEG(angle_)
               << ", last_updated_angle = "
               << IMP_RAD_2_DEG(last_updated_angle_) << std::endl );
      // rotate by the difference from last_updated_angle_
      IMP::algebra::Rotation3D R =
        IMP::algebra::get_rotation_about_normalized_axis
        ( rot_axis_unit_vector_, angle_ - last_updated_angle_ );
      IMP::algebra::Transformation3D R_origin =
        IMP::algebra::get_rotation_about_point(rot_axis_origin_, R);

      // debug prints
      //nice_print_trans(R_origin, "R_origin: ");

      return R_origin;
    }


 protected:
  // the angle in Radians about the joint axis ("unit vector")
  double angle_;

  // the angle used when the child reference frame was last up-to-date
  // mutable cause logically does not change the state of this object
  // (or is that so?)
  mutable double last_updated_angle_;

  // the unit vector around which the joint revolves in parent coords
  IMP::algebra::Vector3D rot_axis_unit_vector_;

  // the joint origin of rotation in parent coords
  IMP::algebra::Vector3D rot_axis_origin_;
};



/********************** DihedralAngleRevoluteJoint ***************/


class  IMPKINEMATICSEXPORT
DihedralAngleRevoluteJoint : public RevoluteJoint{
 public:
  /**
     constructs a dihedral angle that revolves around the axis b-c,
     using a,b,c,d as witnesses for the dihedral angle
     // TODO: do we want to handle derivatives?

     @param parent,child kinematic nodes upstream and downstream (resp.) of
                    this joint
     @param a,b,c,d 'witnesses' whose coordinates define the dihedral
                    angle between the planes containing a-b-c and
                    b-c-d)

     @note It is assumed that neither a, b and c are downstream of child,
           and also that d is not upstream of it
     */
  DihedralAngleRevoluteJoint
    (IMP::core::RigidBody parent, IMP::core::RigidBody child,
     IMP::core::XYZ a, IMP::core::XYZ b, IMP::core::XYZ c, IMP::core::XYZ d);

 protected:
  /**
      updates the rot_axis_unit_vector_ and rot_axis_origin_ variables
      in parent coordinates based on the witnesses b_ and c_,
      using b_-c_ as the axis of rotation
      @it is assumed b_ and c_ have update cartesian coordinates
  */
  virtual void update_axis_of_rotation_from_cartesian_witnesses(){
    using namespace IMP::algebra;
    IMP_USAGE_CHECK
      ( get_distance( b_.get_coordinates(), c_.get_coordinates() )
        > 1e-12 ,
        "witnesses b and c must be non identical beyone numerical error" );
    ReferenceFrame3D rf_parent = get_parent_node().get_reference_frame();
    //nice_print_trans(rf_parent.get_transformation_to(), "Parent trans: ");
    rot_axis_origin_ = rf_parent.get_local_coordinates( b_.get_coordinates() );
    IMP_LOG( VERBOSE, "global b_ " << b_.get_coordinates()
             <<  " and local parent b_ " << rot_axis_origin_ << std::endl);
    Vector3D v =
      rf_parent.get_local_coordinates( c_.get_coordinates() )
      - rf_parent.get_local_coordinates( b_.get_coordinates() );
    rot_axis_unit_vector_ = v.get_unit_vector();
    IMP_LOG( VERBOSE, "local axis of rot unnorm " << v
             <<  " global axis " << c_.get_coordinates() - b_.get_coordinates()
             << std::endl );
  };

  /**
      this protected method uses the cartesian witnesses to compute
      the actual current dihedral angle of this joint (assuming
      external coordinates of required cartesian witnesses are up to
      date)
  */
  virtual double get_current_angle_from_cartesian_witnesses() const;


 private:
    IMP::core::XYZ a_;
    IMP::core::XYZ b_;
    IMP::core::XYZ c_;
    IMP::core::XYZ d_;
};

/********************** BondAngleRevoluteJoint ***************/

class  IMPKINEMATICSEXPORT BondAngleRevoluteJoint : public RevoluteJoint{
 public:
  /**
     constructs a joint that controls the angle a-b-c. The joint
     revolves around the axis that passes through b, normal to the
     plane containing a, b and c. a,b and c are the witnesses for the
     bond angle.
     // TODO: do we want to handle derivatives?

     @param parent,child kinematic nodes upstream and downstream (resp.) of
                    this joint
     @param a,b,c 'witnesses' whose coordinates define the joint angle a-b-c

     @note It is assumed that a and b are upstream of or inside
           this joint's child rigid body, and that c is downstream of
           it or inside it.
  */
  BondAngleRevoluteJoint
    (IMP::core::RigidBody parent, IMP::core::RigidBody child,
     IMP::core::XYZ a, IMP::core::XYZ b, IMP::core::XYZ c);

 protected:

  /**
     this protected method uses the cartesian witnesses to compute the
     actual current bond angle of this joint (assuming external
     coordinates of required cartesian witnesses are up to date)
  */
  virtual double get_current_angle_from_cartesian_witnesses() const;

  /**
     updates the rot_axis_unit_vector_ and rot_axis_origin_ variables
     using b_ as origin of rotation and a vector perpendicular to the
     plane containing a_,b_,c_ as axis of rotation, in global
     coordinates
  */
  virtual void update_axis_of_rotation_from_cartesian_witnesses(){
    using namespace IMP::algebra;

    IMP_USAGE_CHECK
      ( get_distance( b_.get_coordinates(), c_.get_coordinates() )
        > 1e-12 ,
        "witnesses b and c must be non identical beyond numerical error" );
    IMP_USAGE_CHECK
      ( get_distance( b_.get_coordinates(), a_.get_coordinates() )
        > 1e-12 ,
        "witnesses b and a must be non identical beyond numerical error" );
  ReferenceFrame3D rf_parent = get_parent_node().get_reference_frame();
  Vector3D v = rf_parent.get_local_coordinates
    ( get_perpendicular_vector(a_, b_, c_) );
  rot_axis_unit_vector_ = v.get_unit_vector();
  rot_axis_origin_ = rf_parent.get_local_coordinates
    ( b_.get_coordinates() );
  };

 private:

  static IMP::algebra::Vector3D
    get_perpendicular_vector(core::XYZ a, core::XYZ b, core::XYZ c)
  {
    IMP::algebra::Vector3D
      v1 = a.get_coordinates() - b.get_coordinates();
    IMP::algebra::Vector3D
      v2 = c.get_coordinates() - b.get_coordinates();
    return IMP::algebra::get_vector_product(v1, v2);
  }
  IMP::core::XYZ a_;
  IMP::core::XYZ b_;
  IMP::core::XYZ c_;
};



IMP_OBJECTS(RevoluteJoint, RevoluteJoints);
IMP_OBJECTS(DihedralAngleRevoluteJoint, DihedralAngleRevoluteJoints);
IMP_OBJECTS(BondAngleRevoluteJoint, BondAngleRevolteJoints);

IMPKINEMATICS_END_NAMESPACE

#endif  /* IMPKINEMATICS_REVOLUTE_JOINTS_H */
