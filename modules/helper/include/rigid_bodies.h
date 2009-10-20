/**
 *  \file helper/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_RIGID_BODIES_H
#define IMPHELPER_RIGID_BODIES_H

#include "config.h"
#include <IMP/core/rigid_bodies.h>
#include <IMP/atom/Hierarchy.h>

IMPHELPER_BEGIN_NAMESPACE


//! Sets up the ScoreState needed for a rigid body
/**
   \param[in] rbs particles to make into rigid bodies
   \param[in] pr The refiner to get the constituent particles
   \relatesalso IMP::core::RigidBody
   \note The rigid bodies are set to be optimized.
   \note The composition of the rigid bodies may be cached and changes after
   setup may not be detected.


   To make ths body rigid, add the score state to the model.
   To stop keeping the body rigid, remove the returned score state from the
   model.
 */
IMPHELPEREXPORT ScoreState* create_rigid_bodies(SingletonContainer* rbs,
                                                Refiner *pr,
#ifndef IMP_DOXYGEN
                                                bool snapping=false
#endif
);

//! Creates a rigid body and sets up the needed score states
/**
   \param[in] p the particle to create the rigid body using
   \param[in] members The XYZ particles comprising the rigid body
   \relatesalso IMP::core::RigidBody

   \note The rigid body is set to be optimized.
   \note The composition of the rigid bodies may be cached and changes after
   setup may not be detected.

   To make ths body rigid, add the score state to the model.
   To stop keeping the body rigid, remove the returned score state from the
   model.
 */
IMPHELPEREXPORT ScoreState* create_rigid_body(Particle *p,
                                              const core::XYZs &members,
#ifndef IMP_DOXYGEN
                                              bool snapping=false
#endif
);


//! Creates a rigid body and sets up the needed score states
/**
   \param[in] h the atom::Hierarchy to create the rigid body from
   \relatesalso IMP::core::RigidBody

   \note The rigid body is set to be optimized.
   \note The composition of the rigid bodies may be cached and changes after
   setup may not be detected.

   To make ths body rigid, add the score state to the model.
   To stop keeping the body rigid, remove the returned score state from the
   model.

   The representation of the rigid body is the leaves of the passed
   hierarchy. Any internal nodes in the hierarchy which have coordinates
   are added as members. In addition, a radius is added to h so that
   it includes all the leaves.
 */
IMPHELPEREXPORT ScoreState* create_rigid_body(atom::Hierarchy h);


//! Give the rigid body a radius to include its members
/** Make sure that the RigidBody particle has a radius large enough to include
    its reprentation. One cannot use the cover_particles() method as that will
    change the location of the center. The default choice of refiner is
    core::RigidMembersRefiner.
    \relatesalso IMP::core::RigidBody
 */
IMPHELPEREXPORT void cover_rigid_body(core::RigidBody d,
                                      Refiner *r,
                     FloatKey rk= core::XYZR::get_default_radius_key());
/** \see cover_rigid_body(core::RigidBody, Refiner*,FloatKey)

    This version uses the RigidMembersRefiner as a default choice
 */
inline void cover_rigid_body(core::RigidBody d,
                     FloatKey rk= core::XYZR::get_default_radius_key())
{
  IMP::internal::OwnerPointer<Refiner> ref(new core::RigidMembersRefiner());
  cover_rigid_body(d, ref, rk);
}

IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_RIGID_BODIES_H */
