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
/** \see RigidBodyDecorator
    \see RigidMemberDecorator
 */
class IMPCOREEXPORT RigidBodyTraits {
  Pointer<internal::RigidBodyData> d_;
 public:
  //! Create as set of rigid body data using the prefix as an identifier
  /** The radius and mass keys can be default keys, in which case the
      masses will be considered equal and the radii 0, respectively.
  */
  RigidBodyTraits(std::string prefix="default_rigid",
                  FloatKey radius=FloatKey(),
                  FloatKey mass=FloatKey());
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
  bool get_has_required_attributes_for_body(Particle *p) const;
  bool get_has_required_attributes_for_member(Particle* p) const;
  void add_required_attributes_for_body(Particle *p) const;
  void add_required_attributes_for_member(Particle *p) const;
};

class RigidMemberDecorator;

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

  //! Create a new rigid bid
  /** \param[in] p The particle to make into a rigid body
      \param[in] gc The ParticleRefiner which returns the components of
      the rigid body. They must already be XYZDecorator particles.
      \param[in] tr The traits class to use.
      The initial position and orientation of p is computed, as are the
      relative positions of the member particles. The member particles
      do not already need to RigidMemberDecorator particles.
   */
  static RigidBodyDecorator create(Particle *p,
                            ParticleRefiner *gc,
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
    then snaps the members to their rigid locations.

    \verbinclude rigid_bodies.py

    \see RigidBodyDecorator
*/
class IMPCOREEXPORT UpdateRigidBodyOrientation: public SingletonModifier {
  Pointer<ParticleRefiner> pr_;
  RigidBodyTraits tr_;
 public:
  UpdateRigidBodyOrientation(ParticleRefiner *pr,
                             RigidBodyTraits tr= RigidBodyTraits()):
    pr_(pr), tr_(tr){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};

//! Accumulate the derivatives from the refined particles in the rigid body
/** \see RigidBodyDecorator

    \verbinclude rigid_bodies.py
    \see UpdateRigidBodyMembers
 */
class IMPCOREEXPORT AccumulateRigidBodyDerivatives:
  public SingletonModifier {
  Pointer<ParticleRefiner> pr_;
  RigidBodyTraits tr_;
 public:
  AccumulateRigidBodyDerivatives(ParticleRefiner *pr,
                                 RigidBodyTraits tr= RigidBodyTraits()):
    pr_(pr), tr_(tr){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};


//! Compute the coordinates of the RigidMember objects bases on the orientation
/** This should be applied after evaluate to keep the bodies rigid.
    \see RigidBodyDecorator
    \see AccumulateRigidBodyDerivatives */
class IMPCOREEXPORT UpdateRigidBodyMembers: public SingletonModifier {
  Pointer<ParticleRefiner> pr_;
  RigidBodyTraits tr_;
 public:
  UpdateRigidBodyMembers(ParticleRefiner *pr,
                         RigidBodyTraits tr= RigidBodyTraits()):
    pr_(pr), tr_(tr){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODIES_H */
