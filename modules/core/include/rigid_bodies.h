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
#include "internal/core_version_info.h"

#include "XYZDecorator.h"
#include "SingletonContainer.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/ScoreState.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>

IMPCORE_BEGIN_NAMESPACE

//! A class to store rigid body-related data
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
 */
class IMPCOREEXPORT RigidBodyDecorator: public XYZDecorator {
 public:
  IMP_DECORATOR_TRAITS(RigidBodyDecorator, XYZDecorator,
                       RigidBodyTraits, traits,
                       internal::get_default_rigid_body_traits());

  //! Create a new rigid bid
  /** \param[in] p The particle to make into a rigid body
      \param[in] members The particles which will be members of the body.
      They must already be XYZDecorator particles.
      \param[in] tr The traits class to use.
      The initial position and orientation of p is computed, as are the
      relative positions of the member particles. The member particles
      do not already need to RigidMemberDecorator particles.
   */
  static RigidBodyDecorator create(Particle *p,
                            const Particles &members,
              RigidBodyTraits tr= internal::get_default_rigid_body_traits());

  //! Update the coordinates and orientation of the body from the members
  /** The member particles are then snapped to their rigid locations.

      \param[in] members The particles making up the rigid body. All
      the particles must be RigidMemberDecorators.
   */
  void set_transformation(const Particles &members);

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

};

IMP_OUTPUT_OPERATOR(RigidBodyDecorator);


//! A decorator for a particle that is part of a rigid body
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
  void set_coordinates(const algebra::Vector3D &center,
                       const IMP::algebra::Rotation3D &rot) {
    set_coordinates(center+rot.rotate(get_internal_coordinates()));
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



//! Force the passed rigid bodies to stay rigid
/** The positions of the rigid bodies are updated before
    each evaluate call to keep the sub particles in a rigid
    configuration.
 */
class IMPCOREEXPORT RigidBodyScoreState: public ScoreState {
  Pointer<SingletonContainer> ps_;
  Pointer<ParticleRefiner> pr_;
  RigidBodyTraits tr_;
 public:
  //! pass a container of RigidBodyDecorator particles
  RigidBodyScoreState(SingletonContainer *ps,
                      ParticleRefiner *pr,
                      RigidBodyTraits tr= RigidBodyTraits());
  IMP_SCORE_STATE(internal::core_version_info);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODIES_H */
