/**
 *  \file rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODIES_H
#define IMPCORE_RIGID_BODIES_H

#include "config.h"
#include "internal/version_info.h"
#include "internal/rigid_bodies.h"

#include "XYZDecorator.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/Refiner.h>
#include <IMP/ScoreState.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>

IMPCORE_BEGIN_NAMESPACE


class RigidMemberDecorator;
class UpdateRigidBodyOrientation;
class AccumulateRigidBodyDerivatives;
class UpdateRigidBodyMembers;

typedef std::vector<RigidMemberDecorator> RigidMemberDecorators;

//! A decorator for a rigid body
/** A rigid body's orientation is stored using a transformation that
    takes a set of internal coordinates for the member particles
    to the current location of the member particles.

    The body is modeled as a collection of balls of radius and mass
    given by the RigidBodyTraits. I, the inertial tensor, is
    computed and diagonalized, but not stored. It can be stored if
    someone wants it.

    \see RigidMemberDecorator
    \see AccumulateRigidBodyDerivatives
    \see UpdateRigidBodyMembers
    \see UpdateRigidBodyOrientation
 */
class IMPCOREEXPORT RigidBodyDecorator: public XYZDecorator {
#ifndef SWIG
  friend class AccumulateRigidBodyDerivatives;
  friend class UpdateRigidBodyOrientation;
  friend class UpdateRigidBodyMembers;
#endif
  RigidMemberDecorators get_members() const;
  //! Return the location of a member particle given the current position
  /** This method computes the coordinates of p given its internal coordinates
      and the current position and orientation of the rigid body.
   */
  algebra::Vector3D get_coordinates(RigidMemberDecorator p) const;
public:
  IMP_DECORATOR(RigidBodyDecorator, XYZDecorator)

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
                                   const Particles &members);

  ~RigidBodyDecorator();

  //!Return true of the particle is a rigid body
  static bool is_instance_of(Particle *p) {
    return internal::get_has_required_attributes_for_body(p);
  }

  // swig doesn't support using, so the method is wrapped
  //! Get the coordinates of the particle
  algebra::Vector3D get_coordinates() const {
    return XYZDecorator::get_coordinates();
  }

  //! Get the transformation implied by the rigid body
  IMP::algebra::Transformation3D get_transformation() const;

  //! Set the current orientation and translation
  void set_transformation(const IMP::algebra::Transformation3D &tr);

  //! Set whether the rigid body coordinates are optimized
  void set_coordinates_are_optimized(bool tf,
                                     bool snapping=false);

  //! Normalized the quaternion
  void normalize_rotation();

  //! Get the derivatives of the quaternion
  algebra::VectorD<4> get_rotational_derivatives() const;

  //! Get the member particles of the rigid body
  Particles get_member_particles() const;
};

IMP_OUTPUT_OPERATOR(RigidBodyDecorator);


//! A decorator for a particle that is part of a rigid body
/**
   \see RigidBodyDecorator
 */
class IMPCOREEXPORT RigidMemberDecorator: public XYZDecorator {
 public:
  IMP_DECORATOR(RigidMemberDecorator, XYZDecorator);

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

  //! return true if it is a rigid member
  static bool is_instance_of(Particle *p) {
    return internal::get_has_required_attributes_for_member(p);
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
 public:
  UpdateRigidBodyOrientation(){}
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
 public:
  AccumulateRigidBodyDerivatives(){}
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
 public:
  UpdateRigidBodyMembers(){}
  IMP_SINGLETON_MODIFIER(internal::version_info);
};

//! Sets up the ScoreState needed for a rigid body
/**
   \param[in] rbs particles to make into rigid bodies
   \param[in] pr The refiner to get the constituent particles
   \param[in] snapping Whether to use snapping or to optimize the coordinates
   directly
   \relates RigidBodyDecorator
   \note The rigid bodies are set to be optimized.
   \note The composition of the rigid bodies may be cached and changes after
   setup may not be detected.


   To make ths body rigid, add the score state to the model.
   To stop keeping the body rigid, remove the returned score state from the
   model.
 */
IMPCOREEXPORT ScoreState* create_rigid_bodies(SingletonContainer* rbs,
                                              Refiner *pr,
                                              bool snapping=false);

//! Creates a rigid body and sets up the needed score states
/**
   \param[in] m the model
   \param[in] members The XYZ particles comprising the rigid body
   \param[in] snapping Whether to use snapping or to optimize the coordinates
   directly
   \relates RigidBodyDecorator

   \note The rigid body is set to be optimized.
   \note The composition of the rigid bodies may be cached and changes after
   setup may not be detected.

   To make ths body rigid, add the score state to the model.
   To stop keeping the body rigid, remove the returned score state from the
   model.
 */
IMPCOREEXPORT ScoreState* create_rigid_body(Particle *p,
                                            const Particles &members,
                                            bool snapping=false);


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODIES_H */
