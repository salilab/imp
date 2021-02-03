/**
 *  \file IMP/core/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODIES_H
#define IMPCORE_RIGID_BODIES_H

#include <IMP/core/core_config.h>
#include "internal/rigid_bodies.h"

#include "XYZ.h"
#include "XYZR.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/Refiner.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/ReferenceFrame3D.h>
#include <IMP/display/particle_geometry.h>
#include <Eigen/Dense>

IMPCORE_BEGIN_NAMESPACE

IMP_DECORATORS_DECL(RigidMember, RigidMembers);
IMP_DECORATORS_DECL(RigidBodyMember, RigidBodyMembers);

//! A decorator for a rigid body
/** A rigid body particle describes a set of particles, known
    as the members, which move rigidly together. The rigid body
    is represented as an algebra::ReferenceFrame3D coupled
    with local coordinates (RigidMember::get_internal_coordinates())
    for the members expressed in that reference frame. The
    global coordinates of the members are accessed, as with
    other global coordinates, via the XYZ::get_coordinates().

    Since the
    members are simply a set of particles which move together
    they don't (necessarily) define a shape. For example,
    the members of the rigid body made from a molecular hierarchy
    would include particles corresponding to intermediate levels
    of the hierarchy. As a result, methods
    that use rigid bodies usually should simply take the list of
    particles they are interested in and then check for rigid
    bodies internally.

    The initial reference of the rigid body is computed from
    the coordinates, masses and radii of the particles
    passed to the constructor, based on diagonalizing the
    inertial tensor (which is not stored, currently).

    The rigid body radius is the farthest point of any of its
    members from the origin of its reference frame. For rigid
    body members, this takes into account the radius of the
    member.

    RigidBodies can be nested (that is, a RigidBody can have
    another RigidBody as a member). This can be useful for
    organizational reasons as well as for accelerating
    computations since operations are affected by
    the total number of children contained in the rigid body
    being operated on. Examples of this include collision detection
    where if you have multiple representations of geometry at
    different resolutions it is faster to put each of them
    in a separate rigid body and then create one rigid body
    containing all of them.

    It is often desirable to randomize the orientation of a rigid
    body:
    \include randomize_rigid_body.py

    \usesconstraint

    \see RigidMember
    \see NonRigidMember
    \see RigidBodyMover
    \see RigidClosePairsFinder
    \see RigidBodyDistancePairScore
 */
class IMPCOREEXPORT RigidBody : public XYZ {
 private:
  /*  Computes the coordinates of p given its internal (local)
      coordinates and the current position and orientation of the
      rigid body.
   */
  algebra::Vector3D get_coordinates(RigidMember p) const;

  void add_member_internal(Particle *p,
                           const algebra::ReferenceFrame3D &rf);

  //! do updates to rigid body upon changes in its members
  //! such as updating the rigid body radius based on the
  //! point/sphere distance of all of its point/sphere members
  //! from its origin
  void on_change();

  static void teardown_constraints(Particle *p);

  static ObjectKey get_constraint_key_0();

  static ObjectKey get_constraint_key_1();

  // setup rigid body attributes with particles in ps, using their
  // center of mass, inertia tensor  to initialize the reference frame
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                ParticleIndexesAdaptor ps);

  // setup a rigid body with specified reference frame
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf);

  void setup_score_states();

  // add a member associated with xyz coords (if it has a ref frame,
  // it is still being ignored)
  void add_point_member(ParticleIndex pi);

  // add a member associated with a reference frame
  void add_rigid_body_member(ParticleIndex pi);

 public:
  RigidMembers get_rigid_members() const;

  //! Get keys for rotation quaternion.
  static FloatKeys get_rotation_keys() {
    return internal::rigid_body_data().quaternion_;
  }

  //! Returns a list of all members that are not themselves decorated as
  //! rigid bodies, in the form of particle indexes.
  const ParticleIndexes &get_member_particle_indexes() const {
    static ParticleIndexes empty;
    if (get_model()->get_has_attribute(internal::rigid_body_data().members_,
                                       get_particle_index())) {
      return get_model()->get_attribute(internal::rigid_body_data().members_,
                                        get_particle_index());
    } else {
      return empty;
    }
  }

  //! Get all members that are themselves decorated as rigid bodies,
  //! as model particle indexes
  const ParticleIndexes &get_body_member_particle_indexes() const {
    static ParticleIndexes empty;
    if (get_model()->get_has_attribute(
            internal::rigid_body_data().body_members_, get_particle_index())) {
      return get_model()->get_attribute(
          internal::rigid_body_data().body_members_, get_particle_index());
    } else {
      return empty;
    }
  }

  //! Get the particle indexes of any member of this rigid body, regardless
  //! of whether it is itself a rigid body or not
  ParticleIndexes get_member_indexes() const {
    return get_member_particle_indexes() + get_body_member_particle_indexes();
  }

  IMP_DECORATOR_METHODS(RigidBody, XYZ);

  /**
     Create a rigid body for pi with the particle indexes ps as its members.
     The coordinates of pi are set to the center of mass of ps and the rotation
     of its reference frame is based on the diagonalized inertia tensor of ps.

     @note If size(ps)=1, then its reference frame is copied if it is a
           rigid body, or its rotation is set to identity if it is not
           a rigid body.
   */
  IMP_DECORATOR_SETUP_1(RigidBody, ParticleIndexesAdaptor, ps);

  /**
      Create a rigid body with the passed reference frame as its initial
      position.
  */
  IMP_DECORATOR_SETUP_1(RigidBody, algebra::ReferenceFrame3D, rf);

  //! Make the rigid body no longer rigid.
  static void teardown_particle(RigidBody rb);

  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(RigidBody);
  ~RigidBody();

  //! Return true if the particle is a rigid body
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return internal::get_has_required_attributes_for_body(m, pi);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  //! (= translation from local to global rigid body coordinates)
  algebra::Vector3D get_coordinates() const { return XYZ::get_coordinates(); }

  //! returns the rotation of the particle
  //! (= rotation from local to global rigid body orientation)
  IMP::algebra::Rotation3D get_rotation() const {
    return get_reference_frame().get_transformation_to().get_rotation();
  }

  //! Get the reference frame of this rigid body, which enables
  //! trnasformation between the local rigid body coordinates
  //! global coordinates
  IMP::algebra::ReferenceFrame3D get_reference_frame() const {
    algebra::VectorD<4> v(
        get_model()->get_attribute(internal::rigid_body_data().quaternion_[0],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().quaternion_[1],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().quaternion_[2],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().quaternion_[3],
                                   get_particle_index()));
    IMP_USAGE_CHECK_FLOAT_EQUAL(v.get_squared_magnitude(), 1,
                                "Rotation is not a unit vector: " << v);
    /*if (v.get_squared_magnitude() > 0){
      v = v.get_unit_vector();
      } else {
      v = algebra::VectorD<4>(1,0,0,0);
      }*/
    bool assume_normalized = true;
    IMP::algebra::Rotation3D rot(v, assume_normalized);
    return algebra::ReferenceFrame3D(
        algebra::Transformation3D(rot, get_coordinates()));
  }

  //! Set the current reference frame
  /** All members of the rigid body will have their coordinates updated
      immediately.
      \see IMP::core::transform(RigidBody,const algebra::Transformation3D&)
      \see set_reference_frame_lazy()
   */
  void set_reference_frame(const IMP::algebra::ReferenceFrame3D &tr);

  //! Change the reference, delay updating the members until evaluate
  /** \see set_reference_frame()
   */
  inline void set_reference_frame_lazy
    (const IMP::algebra::ReferenceFrame3D &tr)
  {
    algebra::VectorD<4> v =
      tr.get_transformation_to().get_rotation().get_quaternion();
    get_particle()->set_value(internal::rigid_body_data().quaternion_[0], v[0]);
    get_particle()->set_value(internal::rigid_body_data().quaternion_[1], v[1]);
    get_particle()->set_value(internal::rigid_body_data().quaternion_[2], v[2]);
    get_particle()->set_value(internal::rigid_body_data().quaternion_[3], v[3]);
    set_coordinates(tr.get_transformation_to().get_translation());
  }

#ifndef SWIG
#ifndef IMP_DOXYGEN
  //! 'expert' method for setting the reference more quickly
  //! use at own risk
  inline void set_rotation_lazy_using_internal_tables
    (const IMP::algebra::Rotation3D &rot,
     double* quaternion_tables[])
  {
    algebra::VectorD<4> v =
      rot.get_quaternion();
    int pi=get_particle_index().get_index();
    quaternion_tables[0][pi]=v[0];
    quaternion_tables[1][pi]=v[1];
    quaternion_tables[2][pi]=v[2];
    quaternion_tables[3][pi]=v[3];
  }

  //! 'expert' method for setting the reference more quickly
  //! use at own risk
  inline void apply_rotation_lazy_using_internal_tables
    (const IMP::algebra::Rotation3D &rot,
     double* quaternion_tables[])
  {
    int pi=get_particle_index().get_index();
    IMP::algebra::Rotation3D cur_rot
      ( quaternion_tables[0][pi],
        quaternion_tables[1][pi],
        quaternion_tables[2][pi],
        quaternion_tables[3][pi] );
    algebra::VectorD<4> v=
      (cur_rot*rot).get_quaternion();;
    quaternion_tables[0][pi]=v[0];
    quaternion_tables[1][pi]=v[1];
    quaternion_tables[2][pi]=v[2];
    quaternion_tables[3][pi]=v[3];
  }

#endif // IMP_DOXYGEN
#endif // SWIG




  /** Update the reference frame of the rigid body based on aligning
      the current global coordinates of the passed rigid body members
      onto their old local coordinates. Non-passed members are ignored.

      This method is useful for updating the rigid body after new
      global coordinates were loaded for the members. The members are
      passed explicitly since, typically, some are desired to just
      move along with the newly loaded rigid body.

      \note This requires at least three members that are not colinear
      to work.
  */
  void set_reference_frame_from_members(const ParticleIndexes &members);

  //! Pull back global adjoints from members.
  /** Adjoints (reverse-mode sensitivities) are partial derivatives of the
      score with respect to intermediate values in the scoring function
      computation, such as the global coordinates of a bead within a rigid
      body or the global reference frame of a nested rigid body.

      This function pulls back (back-propagates) global adjoints and local
      torque on all members to the global rotation, global coordinates, and
      local torque on this rigid body and the internal coordinates and
      rotation of any non-rigid members.

      This is called by an internal score state after scoring function
      evaluation and is not meant to be called by the user.
   */
  void pull_back_members_adjoints(DerivativeAccumulator &da);

  //! Pull back global adjoints from member that is a point.
  /** 
      @param pi index of member particle
      @param da accumulator for the adjoints
   */
  void pull_back_member_adjoints(ParticleIndex pi,
                                 DerivativeAccumulator &da);

#ifndef SWIG
  /** Same as above, but uses fewer allocations.

      @param pi      index of member particle        
      @param T       transformation from this body's local coordinates to global
      @param x       local coordinates of the member
      @param Dy      adjoint on the member's global coordinates
      @param Dx      adjoint on the member's local coordinates
      @param DT      adjoint on the transformation
      @param xtorque torque contribution from Dy in local coordinates
      @param da      accumulator for the adjoints
   */
  void pull_back_member_adjoints(ParticleIndex pi,
                                 const algebra::Transformation3D &T,
                                 algebra::Vector3D &x,
                                 algebra::Vector3D &Dy,
                                 algebra::Vector3D &Dx,
                                 algebra::Transformation3DAdjoint &DT,
                                 algebra::Vector3D &xtorque,
                                 DerivativeAccumulator &da);
#endif

  //! Pull back global adjoints from member that is also a rigid body.
  /** 
      @param pi index of member particle
      @param da accumulator for the adjoints
   */
  void pull_back_body_member_adjoints(ParticleIndex pi,
                                      DerivativeAccumulator &da);

#ifndef SWIG
  /** Same as above, but uses fewer allocations.

      @param pi         index of member particle        
      @param TA         transformation from this body's local coordinates to global
      @param TB         transformation from member's local coordinates to this
                        body's local coordinates
      @param DTC        adjoint on composition of TA and TB, which is the
                        transformation from the member's local coordinates to
                        global
      @param DTA        adjoint on TA
      @param DTB        adjoint on TB
      @param betatorque torque contribution from DTC in local coordinates at
                        beta, the position of the member in local coordinates.
      @param da         accumulator for the adjoints
   */
  void pull_back_body_member_adjoints(ParticleIndex pi,
                                      const algebra::Transformation3D &TA,
                                      algebra::Transformation3D &TB,
                                      algebra::Transformation3DAdjoint &DTC,
                                      algebra::Transformation3DAdjoint &DTA,
                                      algebra::Transformation3DAdjoint &DTB,
                                      algebra::Vector3D &betatorque,
                                      DerivativeAccumulator &da);
#endif

  /**  Update the translational and rotational derivatives
       on the rigid body center of mass, using the Cartesian derivative
       vector at a speicified location (the point where the force is
       being applied).

       Updates both the quaternion derivatives and the torque.

      @param local_derivative The derivative vector in local rigid body coordinates
      @param local_location   The location where the derivative is taken in local
                              rigid body coordinates
      @param da               Accumulates the output derivative over the rigid body
                              center of mass (translation and rotation torque, quaternion)
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.12)
  void add_to_derivatives(const algebra::Vector3D &local_derivative,
                          const algebra::Vector3D &local_location,
                          DerivativeAccumulator &da);

  /** Faster version of the above, if all is cached.

      @param local_derivative    The derivative vector in local rigid body coordinates
      @param global_derivative   The derivative vector in global coordinates
      @param local_location      The location where the derivative is taken in local
                                 rigid body coordinates
      @param rot_local_to_global Rotation matrix from local rigid body to
                                 global coordinates
      @param da                  Accumulates the output derivative over the rigid body
                                 center of mass (translation and rotation torque, quaternion)
  */
  IMPCORE_DEPRECATED_METHOD_DECL(2.12)
  void add_to_derivatives(const algebra::Vector3D &local_derivative,
                          const algebra::Vector3D &global_derivative,
                          const algebra::Vector3D &local_location,
                          const algebra::Rotation3D &rot_local_to_global,
                          DerivativeAccumulator &da);

  /** Update the rotational derivatives from another body specified by the
      rotation from the other body's local coordinates to this body's local
      coordinates. The provided quaternion derivative on the other body are in
      the reference frame of the other body.

      Updates only quaternion derivatives.

      @param other_qderiv        The derivative on the quaternion taking the other body's
                                 local coordinates to global.
      @param rot_other_to_local  Rotation taking the local coordinates of the other body
                                 to this body's local coordinates.
      @param rot_local_to_global Rotation taking this rigid body's local coordinates to
                                 global coordinates.
      @param da               Accumulates the output derivatives.
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.12)
  void add_to_rotational_derivatives(const algebra::Vector4D &other_qderiv,
                                            const algebra::Rotation3D &rot_other_to_local,
                                            const algebra::Rotation3D &rot_local_to_global,
                                            DerivativeAccumulator &da);

  /** Add to quaternion derivative of this rigid body
      Note that this method does not update the torque.

      @param qderiv  Derivative wrt to quaternion taking local coordinates to
                     global.
      @param da      Object for accumulating derivatives
  */
  inline void add_to_rotational_derivatives(const algebra::Vector4D &qderiv,
                                            DerivativeAccumulator &da);

  /** Add torque to derivative table of this rigid body
      Note that this method does not update the quaternion derivatives, so should
      be used by optimizers that rely on torque only (e.g. BrownianDynamics)

      @param torque_local Torque vector in local reference frame,
                          in units of kCal/Mol/Radian
      @param da           Object for accumulating derivatives
  */
  inline void add_to_torque(const algebra::Vector3D &torque_local,
				DerivativeAccumulator &da);


  /** The units are kCal/Mol/Radian */
  algebra::Vector3D get_torque() const {
    algebra::Vector3D ret;
    for (unsigned int i = 0; i < 3; ++i) {
      ret[i] = get_model()->get_derivative(
          internal::rigid_body_data().torque_[i], get_particle_index());
    }
    return ret;
  }

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
   //! expert method for fast const-access to internal torque
  //! of coordinate #i table
  static double const* access_torque_i_data
    (IMP::Model const* m, unsigned int i)
  {
    IMP_USAGE_CHECK(i<3,"torque is 3 dimensional");
    FloatKey k=
          internal::rigid_body_data().torque_[i];
    double const* ret=m->access_derivative_data(k);
    return ret;
  }

  //! expert method for fast access to internal torque
  //! of coordinate #i table
  static double* access_torque_i_data
    (IMP::Model* m, unsigned int i)
  {
    IMP_USAGE_CHECK(i<3,"torque is 3 dimensional");
    FloatKey k=
          internal::rigid_body_data().torque_[i];
    double* ret=m->access_derivative_data(k);
    return ret;
  }

  //! expert method for fast const-access to internal quaternion coordinate #i table
  static double const* access_quaternion_i_data
    (IMP::Model const* m, unsigned int i)
  {
    IMP_USAGE_CHECK(i<4,"quaternion is 4 dimensional");
    FloatKey k=
          internal::rigid_body_data().quaternion_[i];
    double const* ret=m->FloatAttributeTable::access_attribute_data(k);
    return ret;
  }

  //! expert method for fast access to internal quaternion coordinate #i table
  static double* access_quaternion_i_data
    (IMP::Model* m, unsigned int i)
  {
    IMP_USAGE_CHECK(i<4,"quaternion is 4 dimensional");
    FloatKey k=
          internal::rigid_body_data().quaternion_[i];
    double* ret=m->FloatAttributeTable::access_attribute_data(k);
    return ret;
  }


#endif

  //! Returns true if the rigid body coordinates are flagged as
  //! optimized for Optimizer objects
  bool get_coordinates_are_optimized() const;

  //! Set whether the rigid body coordinates are flagged as optimized
  //! for Optimizer objects
  void set_coordinates_are_optimized(bool tf);

  //! Normalize the quaternion
  void normalize_rotation();

  //! Update the global coordinates of the members based on
  //! their local coordinates and this rigid body's reference frame
  void update_members();

  //! Get the derivatives of the quaternion
  algebra::VectorD<4> get_rotational_derivatives() const;

#ifndef IMP_DOXYGEN
  unsigned int get_number_of_members() const {
    return get_body_member_particle_indexes().size() +
           get_member_particle_indexes().size();
  }

  RigidBodyMember get_member(unsigned int i) const;
#endif
  //! Add a proper member that moves rigidly with this rigid body,
  //! properly handling rigid bodies and XYZ particles.
  /**
     Add p to the list of members. If p is a valid RigidBody, it is added
     as a rigid body member, otherwise it is added as a point member
     (for which the rotation is not tracked). By default, p is considered
     a strictly rigid member, in that its local coordinates are not expected
     to change independently.

     The radius of the rigid body is updated to reflect the new member.

     \see add_non_rigid_member
   */
  void add_member(ParticleIndexAdaptor p);

  /** Add a non-rigid member, for which internal coordinates may change
      independently.

      @note Currently RigidBody non-rigid members are not handled properly.
  */
  void add_non_rigid_member(ParticleIndexAdaptor p);

  /** Set whether a particular member is flagged as a rigid member
      or as a non-rigid member. This affects the way the rigid body
      updates the coordinates and / or reference frame of its members.

     The radius of the rigid body is updated to reflect this change.
  */
  void set_is_rigid_member(ParticleIndex pi, bool tf);
};

#ifndef IMP_DOXYGEN
// inline implementation
void RigidBody::add_to_rotational_derivatives(const algebra::Vector4D &qderiv,
                                              DerivativeAccumulator &da) {
  for (unsigned int i = 0; i < 4; ++i) {
    get_model()->add_to_derivative(internal::rigid_body_data().quaternion_[i],
                                   get_particle_index(), qderiv[i], da);
  }
}


// inline implementation
void RigidBody::add_to_torque(const algebra::Vector3D &torque_local,
                                   DerivativeAccumulator &da) {
  for (unsigned int i = 0; i < 3; ++i) {
    get_model()->add_to_derivative(internal::rigid_body_data().torque_[i],
                                   get_particle_index(), torque_local[i], da);
  }
}

#endif


/** It is often useful to store precalculated properties of the rigid body
    for later use. These need to be cleared out when the rigid body changes.
    To make sure this happens, register the key here.
*/
void IMPCOREEXPORT add_rigid_body_cache_key(ObjectKey k);

//! A member of a rigid body, it has internal (local) coordinates
class IMPCOREEXPORT RigidBodyMember : public XYZ {
  IMP_DECORATOR_METHODS(RigidBodyMember, XYZ);

  RigidBody get_rigid_body() const;

  //! Return the internal (local) coordinates of this member
  /** These coordinates are relative to the reference frame of the
      rigid body that owns it
  */
  const algebra::Vector3D &get_internal_coordinates() const {
    return get_model()->get_internal_coordinates(get_particle_index());
  }

  //! set the internal (local) coordinates for this member
  void set_internal_coordinates(const algebra::Vector3D &v) const {
    get_model()->get_internal_coordinates(get_particle_index()) = v;
    get_rigid_body().get_model()->clear_particle_caches(get_particle_index());
  }

  //! Set the internal (local) coordinates of this member,
  //! assuming it is a rigid body itself
  /** Set the internal (local) coordinates of this rigid body
      relative to the reference frame of the rigid body that owns it
  */
  void set_internal_transformation(const algebra::Transformation3D &v) {
    IMP_USAGE_CHECK(
        get_model()->get_has_attribute(
            internal::rigid_body_data().lquaternion_[0], get_particle_index()),
        "Can only set the internal transformation if member is"
            << " a rigid body itself.");
    set_internal_coordinates(v.get_translation());

    get_model()->set_attribute(internal::rigid_body_data().lquaternion_[0],
                               get_particle_index(),
                               v.get_rotation().get_quaternion()[0]);
    get_model()->set_attribute(internal::rigid_body_data().lquaternion_[1],
                               get_particle_index(),
                               v.get_rotation().get_quaternion()[1]);
    get_model()->set_attribute(internal::rigid_body_data().lquaternion_[2],
                               get_particle_index(),
                               v.get_rotation().get_quaternion()[2]);
    get_model()->set_attribute(internal::rigid_body_data().lquaternion_[3],
                               get_particle_index(),
                               v.get_rotation().get_quaternion()[3]);
    get_rigid_body().get_model()->clear_particle_caches(get_particle_index());
  }

  //! Return the internal (local) coordinates of this member,
  //! assuming it is a rigid body itself
  /** Return the internal (local) coordinates of this rigid body
      relative to the reference frame of the rigid body that owns it
  */
  algebra::Transformation3D get_internal_transformation() const {
    IMP_USAGE_CHECK(
        get_model()->get_has_attribute(
            internal::rigid_body_data().lquaternion_[0], get_particle_index()),
        "Can only set the internal transformation if member is a "
            << "rigid body itself.");
    algebra::Vector3D tr =
        get_model()->get_internal_coordinates(get_particle_index());
    algebra::Rotation3D rot(
        get_model()->get_attribute(internal::rigid_body_data().lquaternion_[0],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().lquaternion_[1],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().lquaternion_[2],
                                   get_particle_index()),
        get_model()->get_attribute(internal::rigid_body_data().lquaternion_[3],
                                   get_particle_index()));
    return algebra::Transformation3D(rot, tr);
  }

  ~RigidBodyMember();
  //! sets the global coordinates of this member using XYZ::set_coordinates()
  // this is here since swig does like using statements
  void set_coordinates(const algebra::Vector3D &center) {
    XYZ::set_coordinates(center);
  }

#ifndef IMP_DOXYGEN
  //! Set the global coordinates from the internal coordinates,
  //! using tr as a reference frame
  void set_coordinates(const algebra::Transformation3D &tr) {
    set_coordinates(tr.get_transformed(get_internal_coordinates()));
  }
#endif
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(RigidBodyMember);

  //! return true if it is a rigid member
  static bool get_is_setup(Model *m, ParticleIndexAdaptor p) {
    return internal::get_has_required_attributes_for_member(m, p);
  }

  static FloatKeys get_internal_coordinate_keys() {
    return internal::rigid_body_data().child_keys_;
  }

  static FloatKeys get_internal_rotation_keys() {
    return internal::rigid_body_data().lquaternion_;
  }
};

//! A decorator for a particle that is part of a rigid body, and is
//! actually rigid
/**
   RigidMember particles, as opposed to NonRigidMember particles, are
   not expected to change their internal (local) coordinates or
   reference frames, and their global coordinates are expected to
   change only through setting the coordinates (or reference frame) of
   the rigid body that owns them.

   \see RigidBody
 */
class IMPCOREEXPORT RigidMember : public RigidBodyMember {
 public:
  IMP_DECORATOR_METHODS(RigidMember, RigidBodyMember);

  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(RigidMember);
  ~RigidMember();

  //! return true if it is a rigid member
  static bool get_is_setup(Model *m, ParticleIndexAdaptor p) {
    return internal::get_has_required_attributes_for_rigid_member(m, p);
  }
};

//! A decorator for a particle that is part of a rigid body but not rigid
/** NonRigidMembers, like RigidMembers, have internal coordinates and move
    along with the rigid body. However, it is expected that their internal
    coordinates will change, and so they are not part of structures that
    assume rigidity.

   \see RigidBody
 */
class IMPCOREEXPORT NonRigidMember : public RigidBodyMember {
 public:
  IMP_DECORATOR_METHODS(NonRigidMember, RigidBodyMember);
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(NonRigidMember);
  ~NonRigidMember();

  //! return true if it is a rigid member
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return internal::get_has_required_attributes_for_non_member(m, p);
  }

  //! Add to derivatives of local coordinates.
  /** @param deriv_parent   The derivative vector in local coordinates of the
                            parent rigid body.
      @param da             Accumulates the derivative over the local translation.
  */
  void add_to_internal_derivatives(const algebra::Vector3D &deriv_parent,
                                   DerivativeAccumulator &da) {
    for (unsigned int i = 0; i < 3; ++i) {
      get_model()->add_to_derivative(get_internal_coordinate_keys()[i],
                                     get_particle_index(), deriv_parent[i], da);
    }
  }

  /** Update the rotational derivatives of the internal transformation.

      Updates only local quaternion derivatives.

      @param local_qderiv        The derivative on the quaternion taking this non-rigid
                                 body's local coordinates to global.
      @param rot_local_to_parent  Rotation taking the local coordinates of the non-rigid
                                  body to its parent's.
      @param rot_parent_to_global Rotation taking the parent rigid body's local coordinates
                                  to global coordinates.
      @param da               Accumulates the output derivatives.
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.12)
  void add_to_internal_rotational_derivatives(
             const algebra::Vector4D &local_qderiv,
             const algebra::Rotation3D &rot_local_to_parent,
             const algebra::Rotation3D &rot_parent_to_global,
             DerivativeAccumulator &da);

  /** Add to internal quaternion derivatives of this non-rigid body

      @param qderiv  Derivative wrt to quaternion taking local coordinates to
                     parent.
      @param da      Object for accumulating derivatives
  */
  void add_to_internal_rotational_derivatives(const algebra::Vector4D &qderiv,
                                              DerivativeAccumulator &da) {
    IMP_USAGE_CHECK(
        get_model()->get_has_attribute(
            get_internal_rotation_keys()[0], get_particle_index()),
        "Can only set derivatives of internal rotation if member is a "
            << "rigid body itself.");
    for (unsigned int i = 0; i < 4; ++i) {
      get_model()->add_to_derivative(get_internal_rotation_keys()[i],
                                     get_particle_index(), qderiv[i], da);
    }
  }


  //! Get derivatives wrt translation component of internal transformation.
  algebra::Vector3D get_internal_derivatives() const {
    algebra::Vector3D ret;
    for (unsigned int i = 0; i < 3; ++i) {
      ret[i] = get_model()->get_derivative(
        get_internal_coordinate_keys()[i], get_particle_index());
    }
    return ret;
  }

  //! Get derivatives wrt quaternion component of internal transformation.
  algebra::Vector4D get_internal_rotational_derivatives() const {
    algebra::Vector4D ret;
    for (unsigned int i = 0; i < 4; ++i) {
      ret[i] = get_model()->get_derivative(
        get_internal_rotation_keys()[i], get_particle_index());
    }
    return ret;
  }
};

#ifndef IMP_DOXYGEN

class IMPCOREEXPORT RigidMembersRefiner : public Refiner {
 public:
  RigidMembersRefiner(std::string name = "RigidMembersRefiner%d")
      : Refiner(name) {}
  virtual bool get_can_refine(Particle *) const IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual const ParticlesTemp get_refined(Particle *) const
      IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidMembersRefiner);
};

namespace internal {
IMPCOREEXPORT RigidMembersRefiner *get_rigid_members_refiner();
}
#endif

//! Transform a rigid body
/** The transformation is applied current conformation of the rigid
    body, as opposed to replacing the current conformation, as in
    RigidBody::set_reference_frame().

    \see RigidBody
    \see algebra::Transformation3D
*/
inline void transform(RigidBody a, const algebra::Transformation3D &tr) {
  a.set_reference_frame(get_transformed(a.get_reference_frame(), tr));
}

/** Compute the rigid body reference frame given a set of input particles.
 */
IMPCOREEXPORT algebra::ReferenceFrame3D get_initial_reference_frame(
    Model *m, const ParticleIndexes &pis);

inline algebra::ReferenceFrame3D get_initial_reference_frame(
    const ParticlesTemp &ps) {
  if (ps.empty()) {
    return algebra::ReferenceFrame3D();
  }
  return get_initial_reference_frame(ps[0]->get_model(),
                                     get_indexes(ps));
}

/** Create a set of rigid bodies that are bound together for efficiency.
    These rigid bodies cannot nest or have other dependencies among them.

    All rigid bodies have the default reference frame.

    \note Do not use this with DOMINO as all the rigid bodies use the same
    ScoreState and so will be considered inter-dependent.
*/
IMPCOREEXPORT ParticlesTemp create_rigid_bodies(Model *m,
                                                        unsigned int n,
                                                        bool no_members =
                                                            false);

IMP_DECORATORS_DEF(RigidMember, RigidMembers);
IMP_DECORATORS(RigidBody, RigidBodies, XYZs);

/** Show the rigid body hierarchy rooted at passed body. */
IMPCOREEXPORT void show_rigid_body_hierarchy(RigidBody rb,
                                             TextOutput out =
                                                 TextOutput(std::cout));

//! Return the index of the outer-most rigid body containing the member.
/** Use this to, for example, group particles into rigid bodies. */
IMPCOREEXPORT ParticleIndex get_root_rigid_body(RigidMember m);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODIES_H */
