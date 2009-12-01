/**
 *  \file core/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODIES_H
#define IMPCORE_RIGID_BODIES_H

#include "config.h"
#include "internal/rigid_bodies.h"

#include "XYZ.h"
#include "XYZR.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/Refiner.h>
#include <IMP/ScoreState.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>

IMPCORE_BEGIN_NAMESPACE


class RigidMember;

typedef Decorators<RigidMember, XYZs> RigidMembers;

//! A decorator for a rigid body
/** A rigid body's orientation is stored using a transformation that
    takes a set of internal coordinates for the member particles
    to the current location of the member particles.

    The body is modeled as a collection of balls of radius and mass
    given by the RigidBodyTraits. I, the inertial tensor, is
    computed and diagonalized, but not stored. It can be stored if
    someone wants it.

    \note In \imp, a rigid body is associated with two, possibly
    identical, sets of particles
    - those particles which transform rigidly along with the rigid
    body, namely the members. The members are accessed through
    methods on the decorator.
    - those particles who make up the detailed representation of
    the rigid body, namely the representation. These are accessed
    through an associated Refiner.

    It is often desirable to randomize the orientation of a rigid
    body:
    \verbinclude randomize_rigid_body.py

    \see RigidMember
    \see AccumulateRigidBodyDerivatives
    \see UpdateRigidBodyMembers
    \see UpdateRigidBodyOrientation
 */
class IMPCOREEXPORT RigidBody: public XYZ {
  //! Return the location of a member particle given the current position
  /** This method computes the coordinates of p given its internal coordinates
      and the current position and orientation of the rigid body.
   */
  algebra::Vector3D get_coordinates(RigidMember p) const;

  void add_member_internal(XYZ d,
                           const algebra::Rotation3D& roti,
                           const algebra::Vector3D& transi, bool cover);
  static RigidBody internal_setup_particle(Particle *p,
                                           const XYZs &members);
  IMP_SCORE_STATE_DECORATOR_DECL(RigidBody);
public:

  RigidMembers get_members() const;

  IMP_DECORATOR(RigidBody, XYZ)

  //! Create a new rigid body, but do not add score states
  /** \param[in] p The particle to make into a rigid body
      \param[in] members The particles to use as members of the rigid body

      The initial position and orientation of p is computed, as are the
      relative positions of the member particles. The member particles
      do not already need to be RigidMember particles, only
      XYZ particles.

      Use the function IMP::core::create_rigid_body() to create the needed
      score states in order to keep the rigid body rigid.
   */
  static RigidBody setup_particle(Particle *p,
                          const XYZs &members);

  //! Create a rigid body based on members of another one
  /** The passed RigidMembers must already be members of
      one rigid body and are used to define the orientation of
      the created RigidBody. It is internally added as a member
      of that RigidBody.
  */
  static RigidBody setup_particle(Particle *p,
                                const RigidMembers &members);

  ~RigidBody();

  //!Return true of the particle is a rigid body
  static bool particle_is_instance(Particle *p) {
    return internal::get_has_required_attributes_for_body(p);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  algebra::Vector3D get_coordinates() const {
    return XYZ::get_coordinates();
  }

  //! Get the transformation implied by the rigid body
  IMP::algebra::Transformation3D get_transformation() const;

  //! Set the current orientation and translation
  /** All members of the rigid body will have their coordinates updated.
      See lazy_set_transformation() if you can weight until evaluate for
      that.
   */
  void set_transformation(const IMP::algebra::Transformation3D &tr);

  //! Change the transformation, but let the score state update the members
  /** See set_transformation()
   */
  void lazy_set_transformation(const IMP::algebra::Transformation3D &tr);

  bool get_coordinates_are_optimized() const;

  //! Set whether the rigid body coordinates are optimized
  void set_coordinates_are_optimized(bool tf);

  //! Normalized the quaternion
  void normalize_rotation();

  //! Update the coordinates of the members
  void update_members();

  //! Get the derivatives of the quaternion
  algebra::VectorD<4> get_rotational_derivatives() const;

  unsigned int get_number_of_members() const;

  RigidMember get_member(unsigned int i) const;

  //! Add a particle as a member
  void add_member(XYZ d);

  //! Add another rigid body
  /** Rigid bodies can be tied together so that one updates
      the other.
  */
  void add_member(RigidBody o);
};

IMP_OUTPUT_OPERATOR(RigidBody);

typedef Decorators<RigidBody, XYZs> RigidBodies;

//! A decorator for a particle that is part of a rigid body
/**
   \see RigidBody
 */
class IMPCOREEXPORT RigidMember: public XYZ {
 public:
  IMP_DECORATOR(RigidMember, XYZ);

  RigidBody get_rigid_body() const;

  //! Return the current orientation of the body
  algebra::Vector3D get_internal_coordinates() const {
    return algebra::Vector3D(get_particle()
                    ->get_value(internal::rigid_body_data().child_keys_[0]),
                    get_particle()
                    ->get_value(internal::rigid_body_data().child_keys_[1]),
                    get_particle()
                    ->get_value(internal::rigid_body_data().child_keys_[2]));
  }

  //! set the internal coordinates for this member
  void set_internal_coordinates(const algebra::Vector3D &v) const {
    get_particle()->set_value(internal::rigid_body_data().child_keys_[0],
                              v[0]);
    get_particle()->set_value(internal::rigid_body_data().child_keys_[1],
                              v[1]);
    get_particle()->set_value(internal::rigid_body_data().child_keys_[2],
                              v[2]);
  }
  //! Member must be a rigid body
  void set_internal_transformation(const  algebra::Transformation3D& v) {
    IMP_USAGE_CHECK(
 get_particle()->has_attribute(internal::rigid_body_data().lquaternion_[0]),
         "Can only set the internal transformation if member is"
         << " a rigid body itself.",
         UsageException);
    get_particle()->set_value(internal::rigid_body_data().child_keys_[0],
                              v.get_translation()[0]);
    get_particle()->set_value(internal::rigid_body_data().child_keys_[1],
                              v.get_translation()[1]);
    get_particle()->set_value(internal::rigid_body_data().child_keys_[2],
                              v.get_translation()[2]);

    get_particle()->set_value(internal::rigid_body_data().lquaternion_[0],
                              v.get_rotation().get_quaternion()[0]);
    get_particle()->set_value(internal::rigid_body_data().lquaternion_[1],
                              v.get_rotation().get_quaternion()[1]);
    get_particle()->set_value(internal::rigid_body_data().lquaternion_[2],
                              v.get_rotation().get_quaternion()[2]);
    get_particle()->set_value(internal::rigid_body_data().lquaternion_[3],
                              v.get_rotation().get_quaternion()[3]);
  }

  algebra::Transformation3D get_internal_transformation() const {
    IMP_USAGE_CHECK(
     get_particle()->has_attribute(internal::rigid_body_data().lquaternion_[0]),
     "Can only set the internal transformation if member is a "
     << "rigid body itself.",
     UsageException);
    algebra::Vector3D
      tr(get_particle()->get_value(internal::rigid_body_data().child_keys_[0]),
         get_particle()->get_value(internal::rigid_body_data().child_keys_[1]),
         get_particle()->get_value(internal::rigid_body_data().child_keys_[2]));
    algebra::Rotation3D
      rot(get_particle()->get_value(internal::rigid_body_data()
                                    .lquaternion_[0]),
          get_particle()->get_value(internal::rigid_body_data()
                                    .lquaternion_[1]),
          get_particle()->get_value(internal::rigid_body_data()
                                    .lquaternion_[2]),
          get_particle()->get_value(internal::rigid_body_data()
                                    .lquaternion_[3]));
    return algebra::Transformation3D(rot, tr);
  }

  //! set the coordinates of the body
  // this is here since swig does like using statements
  void set_coordinates(const algebra::Vector3D &center) {
    XYZ::set_coordinates(center);
  }

  //! Set the coordinates from the internal coordinates
  void set_coordinates(const algebra::Transformation3D &tr) {
    set_coordinates(tr.transform(get_internal_coordinates()));
  }
  ~RigidMember();

  //! return true if it is a rigid member
  static bool particle_is_instance(Particle *p) {
    return internal::get_has_required_attributes_for_member(p);
  }
};

IMP_OUTPUT_OPERATOR(RigidMember);


#ifndef IMP_DOXYGEN
//! A refiner which returns the members of a rigid body
/** You can use this to easily define a rigid body when you want the
    set of particles representing a rigid body to be the same as the
    set of members.
 */
class IMPCOREEXPORT RigidMembersRefiner: public Refiner {
 public:
  RigidMembersRefiner(){}
  IMP_SIMPLE_REFINER(RigidMembersRefiner, get_module_version_info());
};
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  IMPCOREEXPORT RigidMembersRefiner* get_rigid_members_refiner();
}
#endif


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODIES_H */
