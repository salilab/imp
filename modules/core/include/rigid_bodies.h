/**
 *  \file IMP/core/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
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

IMPCORE_BEGIN_NAMESPACE

IMP_DECORATORS_DECL(RigidMember, RigidMembers);

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

  void add_member_internal(kernel::Particle *p,
                           const algebra::ReferenceFrame3D &rf);

  void on_change();

  static void teardown_constraints(kernel::Particle *p);

  static ObjectKey get_constraint_key_0();

  static ObjectKey get_constraint_key_1();

  // setup rigid body attributes with particles in ps, using their
  // center of mass, inertia tensor  to initialize the reference frame
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                kernel::ParticleIndexesAdaptor ps);

  // setup a rigid body with specified reference frame
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf);

  void setup_score_states();

  // add a member associated with xyz coords (if it has a ref frame,
  // it is still being ignored)
  void add_point_member(kernel::ParticleIndex pi);

  // add a member associated with a reference frame
  void add_rigid_body_member(kernel::ParticleIndex pi);

 public:
  /** This method does not return non-rigid members.

      \deprecated_at{2.2} Use get_rigid_members() instead.
    */
  IMPCORE_DEPRECATED_FUNCTION_DECL(2.2)
  RigidMembers get_members() const {
    IMPCORE_DEPRECATED_FUNCTION_DEF(2.2, "Use get_rigid_members() instead.");
    return get_rigid_members();
  }

  RigidMembers get_rigid_members() const;

  //! Returns a list of all members that are not themselves decorated as
  //! rigid bodies, in the form of particle indexes.
  const kernel::ParticleIndexes &get_member_particle_indexes() const {
    static kernel::ParticleIndexes empty;
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
  const kernel::ParticleIndexes &get_body_member_particle_indexes() const {
    static kernel::ParticleIndexes empty;
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
  kernel::ParticleIndexes get_member_indexes() const {
    return get_member_particle_indexes() + get_body_member_particle_indexes();
  }

  IMP_DECORATOR_METHODS(RigidBody, XYZ);

  /**
     Create a rigid body for pi with the particle indexes ps as its members.
     The coordinates of pi are set to the center of mass of ps and the rotation
     of its reference frame is based on the diagonalized inertia tensor of ps.

     @note If size(ps)=1, then its reference frame is copied if it is a rigid body, \
     or its rotation is set to identity if it is not a rigid body.

   */
  IMP_DECORATOR_SETUP_1(RigidBody, kernel::ParticleIndexesAdaptor, ps);

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
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return internal::get_has_required_attributes_for_body(m, pi);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  algebra::Vector3D get_coordinates() const { return XYZ::get_coordinates(); }

  //! Get the reference frame for the local coordinates
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
      \see lazy_set_reference_frame()
   */
  void set_reference_frame(const IMP::algebra::ReferenceFrame3D &tr);

  //! Change the reference, delay updating the members until evaluate
  /** \see set_reference_frame()
   */
  void set_reference_frame_lazy(const IMP::algebra::ReferenceFrame3D &tr);

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
  void set_reference_frame_from_members(const kernel::ParticleIndexes &members);

#ifndef IMP_DOXYGEN
  /** This takes a Cartesian derivative in global coordinates,
      and a location in internal coordinates.

      It is currently hidden since the function signature is highly ambiguous.
   */
  inline void add_to_derivatives(const algebra::Vector3D &derivative,
                          const algebra::Vector3D &local_location,
                          DerivativeAccumulator &da);

  // faster if all is cached
  inline void add_to_derivatives(const algebra::Vector3D &derivative,
                          const algebra::Vector3D &global_derivative,
                          const algebra::Vector3D &local_location,
                          const algebra::Rotation3D &rot,
                          DerivativeAccumulator &da);
#endif

  /** The units are kCal/Mol/Radian */
  algebra::Vector3D get_torque() const {
    algebra::Vector3D ret;
    for (unsigned int i = 0; i < 3; ++i) {
      ret[i] = get_model()->get_derivative(
          internal::rigid_body_data().torque_[i], get_particle_index());
    }
    return ret;
  }

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

  RigidMember get_member(unsigned int i) const;
#endif
  //! Add a proper member that moves rigidly with this rigid body,
  //! properly handling rigid bodies and XYZ particles.
  /**
     Add p to the list of members. If p is a valid RigidBody, it is added
     as a rigid body member, otherwise it is added as a point member
     (for which the rotation is not tracked). By default, p is considered
     a strictly rigid member, in that its local coordinates are not expected
     to change independently.

     \see add_non_rigid_member
   */
  void add_member(kernel::ParticleIndexAdaptor p);

  /** Add a non-rigid member, for which internal coordinates may change
      independently.

      @note Currently RigidBody non-rigid members are not handled properly.
  */
  void add_non_rigid_member(kernel::ParticleIndex pi);

  /** Set whether a particular member is flagged as a rigid member
      or as a non-rigid member. This affects the way the rigid body
      updates the coordinates and / or reference frame of its members.
  */
  void set_is_rigid_member(kernel::ParticleIndex pi, bool tf);
};


// inline implementation
void RigidBody::add_to_derivatives(const algebra::Vector3D &deriv_local,
                                   const algebra::Vector3D &deriv_global,
                                   const algebra::Vector3D &local,
                                   const algebra::Rotation3D &rot,
                                   DerivativeAccumulator &da) {
  // const algebra::Vector3D deriv_global= rot*deriv_local;
  // IMP_LOG_TERSE( "Accumulating rigid body derivatives" << std::endl);
  algebra::VectorD<4> q(0, 0, 0, 0);
  for (unsigned int j = 0; j < 4; ++j) {
    algebra::Vector3D v = rot.get_derivative(local, j);
    q[j] = deriv_global * v;
  }
  XYZ::add_to_derivatives(deriv_global, da);
  for (unsigned int j = 0; j < 4; ++j) {
    get_model()->add_to_derivative(internal::rigid_body_data().quaternion_[j],
                                   get_particle_index(), q[j], da);
  }
  algebra::Vector3D torque = algebra::get_vector_product(local, deriv_local);
  for (unsigned int i = 0; i < 3; ++i) {
    get_model()->add_to_derivative(internal::rigid_body_data().torque_[i],
                                   get_particle_index(), torque[i], da);
  }
}

// inline implementation
void RigidBody::add_to_derivatives(const algebra::Vector3D &deriv_local,
                                   const algebra::Vector3D &local,
                                   DerivativeAccumulator &da) {
  algebra::Rotation3D rot =
      get_reference_frame().get_transformation_to().get_rotation();
  const algebra::Vector3D deriv_global = rot * deriv_local;
  add_to_derivatives(deriv_local, deriv_global, local, rot, da);
}



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
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndexAdaptor p) {
    return internal::get_has_required_attributes_for_member(m, p);
  }

  static FloatKeys get_internal_coordinate_keys() {
    return internal::rigid_body_data().child_keys_;
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
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndexAdaptor p) {
    return internal::get_has_required_attributes_for_rigid_member(m, p);
  }
};

//! A decorator for a particle that is part of a rigid body but not rigid
/** NonRigidMembers, like RigidMembers have internal coordinates and move
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
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex p) {
    return internal::get_has_required_attributes_for_non_member(m, p);
  }
};

#ifndef IMP_DOXYGEN

class IMPCOREEXPORT RigidMembersRefiner : public Refiner {
 public:
  RigidMembersRefiner(std::string name = "RigidMembersRefiner%d")
      : Refiner(name) {}
  virtual bool get_can_refine(kernel::Particle *) const IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
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
    kernel::Model *m, const kernel::ParticleIndexes &pis);

inline algebra::ReferenceFrame3D get_initial_reference_frame(
    const kernel::ParticlesTemp &ps) {
  if (ps.empty()) {
    return algebra::ReferenceFrame3D();
  }
  return get_initial_reference_frame(ps[0]->get_model(),
                                     kernel::get_indexes(ps));
}

/** Create a set of rigid bodies that are bound together for efficiency.
    These rigid bodies cannot nest or have other dependencies amongst them.

    All rigid bodies have the default reference frame.

    \note Do not use this with DOMINO as all the rigid bodies use the same
    ScoreState and so will be considered inter-dependent.
*/
IMPCOREEXPORT kernel::ParticlesTemp create_rigid_bodies(kernel::Model *m,
                                                        unsigned int n,
                                                        bool no_members =
                                                            false);

IMP_DECORATORS_DEF(RigidMember, RigidMembers);
IMP_DECORATORS(RigidBody, RigidBodies, XYZs);

/** Show the rigid body hierarchy rooted at passed body. */
IMPCOREEXPORT void show_rigid_body_hierarchy(RigidBody rb,
                                             base::TextOutput out =
                                                 base::TextOutput(std::cout));

//! Return the index of the outer-most rigid body containing the member.
/** Use this to, for example, group particles into rigid bodies. */
IMPCOREEXPORT kernel::ParticleIndex get_root_rigid_body(RigidMember m);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODIES_H */
