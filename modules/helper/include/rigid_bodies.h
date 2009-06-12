/**
 *  \file helper/rigid_bodies.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_RIGID_BODIES_H
#define IMPHELPER_RIGID_BODIES_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/core/rigid_bodies.h>

IMPHELPER_BEGIN_NAMESPACE


//! Sets up the ScoreState needed for a rigid body
/**
   \param[in] rbs particles to make into rigid bodies
   \param[in] pr The refiner to get the constituent particles
   \param[in] snapping Whether to use snapping or to optimize the coordinates
   directly
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
                                              bool snapping=false);

//! Creates a rigid body and sets up the needed score states
/**
   \param[in] p the particle to create the rigid body using
   \param[in] members The XYZ particles comprising the rigid body
   \param[in] snapping Whether to use snapping or to optimize the coordinates
   directly
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
                                              bool snapping=false);


//! Give the rigid body a radius to include its members
/** Make sure that the RigidBody particle has a radius large enough to include
    its members. One cannot use the cover_particles() method as that will
    change the location of the center.
    \relatesalso IMP::core::RigidBody
 */
IMPHELPEREXPORT void cover_members(core::RigidBody d,
                     FloatKey rk= core::XYZR::get_default_radius_key());

IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_RIGID_BODIES_H */
