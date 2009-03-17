/**
 *  \file rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODIES_H
#define IMPCORE_RIGID_BODIES_H

#include "config.h"
#include "internal/rigid_bodies.h"
#include "internal/version_info.h"

#include "XYZDecorator.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/ParticleRefiner.h>
#include <IMP/ScoreState.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>

IMPCORE_BEGIN_NAMESPACE

//! A class to store rigid body-related data
/** The data includes a prefix to make the keys unique (so that a particle can
    be part of more than one rigid body) as well the the particle refiner to use
    to map between the rigid body and its members.

    \see RigidBodyDecorator
    \see RigidMemberDecorator
    \see RigidBodyDecorator::get_default_traits()
 */
class IMPCOREEXPORT RigidBodyTraits {
  Pointer<internal::RigidBodyData> d_;
 public:
  RigidBodyTraits(){}
  //! Create as set of rigid body data using the prefix as an identifier
  /** The radius and mass keys can be default keys, in which case the
      masses will be considered equal and the radii 0, respectively.

     \note  Having non-default values is not well tested at this point.
  */
  RigidBodyTraits(ParticleRefiner *pr,
                  std::string prefix,
                  FloatKey radius=FloatKey(),
                  FloatKey mass=FloatKey(),
                  bool snap=false);
  //! Get the keys used to store the offset coordinates
  const FloatKeys& get_local_coordinate_keys() const {
    return d_->child_keys_;
  }
  //! Get the keys used to store the offset quaternion
  const FloatKeys& get_quaternion_keys() const {
    return d_->quaternion_;
  }
  //! Get the key used to store the radius
  FloatKey get_radius_key() const {
    return d_->radius_;
  }
  //! Get the key used to store the radius
  FloatKey get_mass_key() const {
    return d_->mass_;
  }
  //! Return the particle refiner used to get the constituent particles
  ParticleRefiner *get_particle_refiner() const {
    return d_->pr_;
  }
  //! Get whether to use snapping or direct optimization of the rigid body
  bool get_snapping() const {
    return d_->snap_;
  }
  //! For testing only currently
  /** This method changes whether the current traits uses snapping or not.
      Changing this setting will not effect existing rigid bodies and passing
      traits whose contents don't match the value when the rigid body was
      created can mess things up.
   */
  void set_snapping(bool tf) {
    d_->snap_=tf;
  }
  //! Set appropriate ranges for contrainable values
  /** Quaternion ranges are set to be from 0 to 1.
   */
  void set_model_ranges(Model *m) const;
  bool get_has_required_attributes_for_body(Particle *p) const;
  bool get_has_required_attributes_for_member(Particle* p) const;
  void add_required_attributes_for_body(Particle *p) const;
  void add_required_attributes_for_member(Particle *p) const;

  //! print stuff
  void show(std::ostream &out) const {
    out << "rigid body traits: " << get_quaternion_keys()[0];
  }
};

IMP_OUTPUT_OPERATOR(RigidBodyTraits);

class RigidMemberDecorator;

typedef std::vector<RigidMemberDecorator> RigidMemberDecorators;

//! A decorator for a rigid body
/** The body is modeled as a collection of balls of radius and mass
    given by the RigidBodyTraits. I, the inertial tensor, is
    computed and diagonalized, but not stored. It can be stored if
    someone wants it.

    \see RigidMemberDecorator
    \see AccumulateRigidBodyDerivatives
    \see UpdateRigidBodyMembers
    \see UpdateRigidBodyOrientation
 */
class IMPCOREEXPORT RigidBodyDecorator: public XYZDecorator {
 public:
  IMP_DECORATOR_TRAITS(RigidBodyDecorator, XYZDecorator,
                       RigidBodyTraits, traits,
                       internal::get_default_rigid_body_traits());

  //! Create a new rigid body, but do not add score states
  /** \param[in] p The particle to make into a rigid body
      \param[in] tr The traits class to use.
      The initial position and orientation of p is computed, as are the
      relative positions of the member particles. The member particles
      do not already need to RigidMemberDecorator particles.

      Use the function IMP::core::create_rigid_body() to add the needed
      score states to the model.
   */
  static RigidBodyDecorator create(Particle *p,
              RigidBodyTraits tr= internal::get_default_rigid_body_traits());

  ~RigidBodyDecorator();

  //!Return true of the particle is a rigid body
  static bool is_instance_of(Particle *p,
                             RigidBodyTraits traits
                             =internal::get_default_rigid_body_traits()) {
    return traits.get_has_required_attributes_for_body(p);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  algebra::Vector3D get_coordinates() const {
    return XYZDecorator::get_coordinates();
  }

  //! Return the location of a member particle given the current position
  /** This method computes the coordinates of p given its internal coordinates
      and the current position and orientation of the rigid body.
   */
  algebra::Vector3D get_coordinates(RigidMemberDecorator p) const;

  //! Get the transformation implied by the rigid body
  IMP::algebra::Transformation3D get_transformation() const;

  //! Set the current orientation and translation
  void set_transformation(const IMP::algebra::Transformation3D &tr);

  //! Set whether the rigid body coordinates are optimized
  void set_coordinates_are_optimized(bool tf);

  //! Normalized the quaternion
  void normalize_rotation();

  //! Get the derivatives of the quaternion
  algebra::VectorD<4> get_rotational_derivatives() const;

  //! Get the member particles of the rigid body
  RigidMemberDecorators get_members() const;

  //! Return the default traits
  /** These use a ChildrenHierarchyDecorator with a
      HierarchyDecorator::get_default_traits() traits.
   */
  static RigidBodyTraits get_default_traits() {
    return internal::get_default_rigid_body_traits();
  }
};

IMP_OUTPUT_OPERATOR(RigidBodyDecorator);


//! A decorator for a particle that is part of a rigid body
/**
   \see RigidBodyDecorator
 */
class IMPCOREEXPORT RigidMemberDecorator: public XYZDecorator {
 public:
  IMP_DECORATOR_TRAITS(RigidMemberDecorator, XYZDecorator,
                       RigidBodyTraits, traits,
                       internal::get_default_rigid_body_traits());

  //! Return the current orientation of the body
  algebra::Vector3D get_internal_coordinates() const {
    return algebra::Vector3D(get_particle()
                    ->get_value(get_traits().get_local_coordinate_keys()[0]),
                    get_particle()
                    ->get_value(get_traits().get_local_coordinate_keys()[1]),
                    get_particle()
                    ->get_value(get_traits().get_local_coordinate_keys()[2]));
  }

  //! set the internal coordinates for this member
  void set_internal_coordinates(const algebra::Vector3D &v) const {
    get_particle()->set_value(get_traits().get_local_coordinate_keys()[0],
                              v[0]);
    get_particle()->set_value(get_traits().get_local_coordinate_keys()[1],
                              v[1]);
    get_particle()->set_value(get_traits().get_local_coordinate_keys()[2],
                              v[2]);
  }

  //! set the coordinates of the body
  // this is here since swig does like using statements
  void set_coordinates(const algebra::Vector3D &center) {
    XYZDecorator::set_coordinates(center);
  }

  //! Set the coordinates from the internal coordinates
  void set_coordinates(const algebra::Transformation3D &tr) {
    set_coordinates(tr.transform(get_internal_coordinates()));
  }
  ~RigidMemberDecorator();

  //! return the mass or 1 if there is no mass key defined
  Float get_mass() const {
    if (get_traits().get_mass_key() != FloatKey()) {
      return get_particle()->get_value(get_traits().get_mass_key());
    } else return 1;
  }

  //! return the radius or 0 if there is no radius key defined
  Float get_radius() const {
    if (get_traits().get_radius_key() != FloatKey()) {
      return get_particle()->get_value(get_traits().get_radius_key());
    } else return 0;
  }

  //! return true if it is a rigid member
  static bool is_instance_of(Particle *p,
                             RigidBodyTraits traits
                             =internal::get_default_rigid_body_traits()) {
    return traits.get_has_required_attributes_for_member(p);
  }
};

IMP_OUTPUT_OPERATOR(RigidMemberDecorator);

//! Compute the orientation of the rigid body from the refined particles
/** This should be applied before evaluate to keep the bodies rigid. It
    computes the optimal orientation given the position of the members and
    then snaps the members to their rigid locations. You can
    use the setup_rigid_bodies and setup_rigid_body methods instead of
    creating these objects yourself.
    \see setup_rigid_bodies
    \see setup_rigid_body
    \verbinclude rigid_bodies.py

    \see RigidBodyDecorator
*/
class IMPCOREEXPORT UpdateRigidBodyOrientation: public SingletonModifier {
  RigidBodyTraits tr_;
 public:
  UpdateRigidBodyOrientation(RigidBodyTraits tr
                             = internal::get_default_rigid_body_traits()):
    tr_(tr){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};

//! Accumulate the derivatives from the refined particles in the rigid body
/** You can
    use the setup_rigid_bodies and setup_rigid_body methods instead of
    creating these objects yourself.
    \see setup_rigid_bodies
    \see setup_rigid_body
    \see RigidBodyDecorator
    \verbinclude rigid_bodies.py
    \see UpdateRigidBodyMembers
 */
class IMPCOREEXPORT AccumulateRigidBodyDerivatives:
  public SingletonModifier {
  RigidBodyTraits tr_;
 public:
  AccumulateRigidBodyDerivatives(RigidBodyTraits tr
                                 =internal::get_default_rigid_body_traits()):
    tr_(tr){}
  IMP_SINGLETON_MODIFIER_DA(internal::version_info);
};


//! Compute the coordinates of the RigidMember objects bases on the orientation
/** This should be applied after evaluate to keep the bodies rigid. You can
    use the setup_rigid_bodies and setup_rigid_body methods instead of
    creating these objects yourself.
    \see setup_rigid_bodies
    \see setup_rigid_body
    \see RigidBodyDecorator
    \see AccumulateRigidBodyDerivatives */
class IMPCOREEXPORT UpdateRigidBodyMembers: public SingletonModifier {
  RigidBodyTraits tr_;
 public:
  UpdateRigidBodyMembers(RigidBodyTraits tr
                         =internal::get_default_rigid_body_traits()):
    tr_(tr){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};

//! Sets up the ScoreState needed for a rigid body
/**
   \param[in] m the model
   \param[in] rbs particles to make into rigid bodies
   \param[in] tr the rigid body traits to use
   \param[in] snap whether to use snapping (as opposed to direct optimization
   of the rotational degrees for freedom).
   \relates RigidBodyDecorator
   \note The rigid bodies are set to be optimized.
 */
IMPCOREEXPORT void create_rigid_bodies(SingletonContainer* rbs,
                                       RigidBodyTraits tr
                                   =internal::get_default_rigid_body_traits());

//! Creates a rigid body and sets up the needed score states
/**
   \param[in] m the model
   \param[in] tr the rigid body traits to use
   \param[in] snap whether to use snapping (as opposed to direct optimization
   of the rotational degrees for freedom).
   \relates RigidBodyDecorator

   \note The rigid body is set to be optimized.
 */
IMPCOREEXPORT RigidBodyDecorator create_rigid_body(Particle *p,
                                                   RigidBodyTraits tr
                               =internal::get_default_rigid_body_traits());


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODIES_H */
