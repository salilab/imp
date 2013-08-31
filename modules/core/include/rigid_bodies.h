/**
 *  \file IMP/core/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODIES_H
#define IMPCORE_RIGID_BODIES_H

#include <IMP/core/core_config.h>
#include "internal/rigid_bodies.h"
#include <IMP/refiner_macros.h>

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
    computations since since operations are affected by
    the total number of children contained in the rigid body
    being operated on. Examples of this include collision detection
    where if you have multiple representations of geometry at
    different resolutions it is faster to put each of them
    in a separate rigid body and then creat one rigid body
    containing all of them.

    It is often desirable to randomize the orientation of a rigid
    body:
    \include randomize_rigid_body.py

    \usesconstraint

    \see RigidMember
    \see RigidBodyMover
    \see RigidClosePairsFinder
    \see RigidBodyDistancePairScore
 */
class IMPCOREEXPORT RigidBody : public XYZ {
  //! Return the location of a member particle given the current position
  /** This method computes the coordinates of p given its internal coordinates
      and the current position and orientation of the rigid body.
   */
  algebra::Vector3D get_coordinates(RigidMember p) const;

  void add_member_internal(kernel::Particle *p, const algebra::ReferenceFrame3D &rf);
  void on_change();
  static void teardown_constraints(kernel::Particle *p);
  static ObjectKey get_constraint_key_0();
  static ObjectKey get_constraint_key_1();

  static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                kernel::ParticleIndexesAdaptor ps);

  static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                const algebra::ReferenceFrame3D &rf);

 public:
  RigidMembers get_members() const;

  //! Return the members as particle pointers
  /** This member function is here for efficiency.*/
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

  IMP_DECORATOR_METHODS(RigidBody, XYZ);
  IMP_DECORATOR_SETUP_1(RigidBody, kernel::ParticleIndexesAdaptor, ps);
  /** Create a rigid body with the passed reference frame as its initial
      position. */
  IMP_DECORATOR_SETUP_1(RigidBody, algebra::ReferenceFrame3D, rf);

  //! Make the rigid body no longer rigid.
  static void teardown_particle(RigidBody rb);

  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(RigidBody);
  ~RigidBody();

  /** Return true of the particle is a rigid body */
  static bool get_is_setup(Model *m, kernel::ParticleIndex pi) {
    return internal::get_has_required_attributes_for_body(m, pi);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  algebra::Vector3D get_coordinates() const { return XYZ::get_coordinates(); }

  //! Get the reference frame for the local coordinates
  IMP::algebra::ReferenceFrame3D get_reference_frame() const {
    algebra::VectorD<4>
      v(get_model()->get_attribute(internal::rigid_body_data().quaternion_[0],
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
    IMP::algebra::Rotation3D rot(v);
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
  /** See set_reference_frame()
   */
  void set_reference_frame_lazy(const IMP::algebra::ReferenceFrame3D &tr);

  /** Update the reference frame of the rigid body based on the current
      coordinates of the passed members (nonpassed members are ignored).
      This can be used to update the rigid
      body after new coordinates were loaded for the members. The members
      are passed explictily since, typically, some are desired to just
      move along with the newly loaded rigid body.

      \note This requires at least three members that are not collinear
      to work.
  */
  void set_reference_frame_from_members(const kernel::ParticleIndexes &members);

#ifndef IMP_DOXYGEN
  /** This takes a cartesian derivative, and a location in internal coordinates.

      It is currently hidden since the function signature is highly ambiguous.
   */
  void add_to_derivatives(const algebra::Vector3D &derivative,
                          const algebra::Vector3D &local_location,
                          DerivativeAccumulator &da);

  void add_to_derivatives(const algebra::Vector3D &derivative,
                          const algebra::Vector3D &global_derivative,
                          const algebra::Vector3D &local_location,
                          const algebra::Rotation3D &rot,
                          DerivativeAccumulator &da);
#endif

  /** The units are kCal/Mol/Radian */
  algebra::Vector3D get_torque() const {
    algebra::Vector3D ret;
    for (unsigned int i = 0; i < 3; ++i) {
      ret[i] =
        get_model()->get_derivative(internal::rigid_body_data().torque_[i],
                                    get_particle_index());
    }
    return ret;
  }

  bool get_coordinates_are_optimized() const;

  //! Set whether the rigid body coordinates are optimized
  void set_coordinates_are_optimized(bool tf);

  //! Normalized the quaternion
  void normalize_rotation();

  //! Update the coordinates of the members
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
  /** Add a member, properly handle rigid bodies and XYZ particles.
   */
  void add_member(kernel::ParticleIndexAdaptor p);

  /** Add a NonRigidMember. Currently RigidBody non-rigid members are
      not handler properly.*/
  void add_non_rigid_member(kernel::ParticleIndex pi);

  /** Set whether a particular member is a rigid member or a non-rigid member.*/
  void set_is_rigid_member(kernel::ParticleIndex pi, bool tf);
};

/** It is often useful to store precalculated properties of the rigid body
    for later use. These need to be cleared out when the rigid body changes.
    To make sure this happens, register the key here.
*/
void IMPCOREEXPORT add_rigid_body_cache_key(ObjectKey k);

//! A decorator for a particle that is part of a rigid body
/**
   \see RigidBody
 */
class IMPCOREEXPORT RigidMember : public XYZ {
 public:
  IMP_DECORATOR_METHODS(RigidMember, XYZ);

  RigidBody get_rigid_body() const;

  //! Return the current orientation of the body
  const algebra::Vector3D &get_internal_coordinates() const {
    return get_model()->get_internal_coordinates(get_particle_index());
  }

  //! set the internal coordinates for this member
  void set_internal_coordinates(const algebra::Vector3D &v) const {
    get_model()->get_internal_coordinates(get_particle_index()) = v;
    get_rigid_body().get_model()->clear_particle_caches(get_particle_index());
  }
  //! Member must be a rigid body
  void set_internal_transformation(const algebra::Transformation3D &v) {
    IMP_USAGE_CHECK(get_model()->get_has_attribute(
                               internal::rigid_body_data().lquaternion_[0],
                                                   get_particle_index()),
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

  algebra::Transformation3D get_internal_transformation() const {
    IMP_USAGE_CHECK(get_model()->get_has_attribute(
                        internal::rigid_body_data().lquaternion_[0],
                               get_particle_index()),
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

  //! XYZ::set_coordiantes()
  // this is here since swig does like using statements
  void set_coordinates(const algebra::Vector3D &center) {
    XYZ::set_coordinates(center);
  }
#ifndef IMP_DOXYGEN
  //! Set the coordinates from the internal coordinates
  void set_coordinates(const algebra::Transformation3D &tr) {
    set_coordinates(tr.get_transformed(get_internal_coordinates()));
  }
#endif
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(RigidMember);
  ~RigidMember();

  //! return true if it is a rigid member
  static bool get_is_setup(Model *m, kernel::ParticleIndexAdaptor p) {
    return internal::get_has_required_attributes_for_member(m, p);
  }

  static FloatKeys get_internal_coordinate_keys() {
    return internal::rigid_body_data().child_keys_;
  }
};

//! A decorator for a particle that is part of a rigid body but not rigid
/** NonRigidMembers, like RigidMembers have internal coordinates and move
    along with the rigid body. However, it is expected that their internal
    coordinates will change, and so they are not part of structures that
    assume rigidity.

   \see RigidBody
 */
class IMPCOREEXPORT NonRigidMember : public XYZ {
 public:
  IMP_DECORATOR_METHODS(NonRigidMember, XYZ);

  RigidBody get_rigid_body() const;

  //! Return the current orientation of the body
  const algebra::Vector3D &get_internal_coordinates() const {
    return get_model()->get_internal_coordinates(get_particle_index());
  }

  //! set the internal coordinates for this member
  void set_internal_coordinates(const algebra::Vector3D &v) const {
    get_model()->get_internal_coordinates(get_particle_index()) = v;
    get_model()->clear_particle_caches(get_particle_index());
  }

  //! Member must be a rigid body
  void set_internal_transformation(const algebra::Transformation3D &v) {
    IMP_USAGE_CHECK(get_model()->get_has_attribute(
                        internal::rigid_body_data().lquaternion_[0],
                               get_particle_index()),
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

  algebra::Transformation3D get_internal_transformation() const {
    IMP_USAGE_CHECK(get_model()->get_has_attribute(
                        internal::rigid_body_data().lquaternion_[0],
                        get_particle_index()),
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

  //! XYZ::set_coordiantes()
  // this is here since swig does like using statements
  void set_coordinates(const algebra::Vector3D &center) {
    XYZ::set_coordinates(center);
  }

#ifndef IMP_DOXYGEN
  //! Set the coordinates from the internal coordinates
  void set_coordinates(const algebra::Transformation3D &tr) {
    set_coordinates(tr.get_transformed(get_internal_coordinates()));
  }
#endif
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(NonRigidMember);
  ~NonRigidMember();

  //! return true if it is a rigid member
  static bool get_is_setup(Model *m, kernel::ParticleIndex p) {
    return internal::get_has_required_attributes_for_non_member(m, p);
  }

  static FloatKeys get_internal_coordinate_keys() {
    return internal::rigid_body_data().child_keys_;
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
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *m,
                                         const kernel::ParticleIndexes &pis) const
      IMP_OVERRIDE;
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

    See RigidBody
     algebra::Transformation3D
*/
inline void transform(RigidBody a, const algebra::Transformation3D &tr) {
  a.set_reference_frame(get_transformed(a.get_reference_frame(), tr));
}

/** Compute the rigid body reference frame given a set of input particles.
 */
IMPCOREEXPORT algebra::ReferenceFrame3D
get_initial_reference_frame(Model *m, const kernel::ParticleIndexes &pis);

inline algebra::ReferenceFrame3D
get_initial_reference_frame(const kernel::ParticlesTemp &ps) {
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
IMPCOREEXPORT kernel::ParticlesTemp create_rigid_bodies(Model *m, unsigned int n,
                                                bool no_members = false);

IMP_DECORATORS_DEF(RigidMember, RigidMembers);
IMP_DECORATORS(RigidBody, RigidBodies, XYZs);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_RIGID_BODIES_H */
