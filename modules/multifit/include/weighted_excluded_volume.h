/**
 *  \file IMP/multifit/weighted_excluded_volume.h
 *  \brief handles low resolution weighted excluded
 *         volume calculation.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H
#define IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/kernel/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/atom/Mass.h>
#include <IMP/multifit/multifit_config.h>
IMPMULTIFIT_BEGIN_NAMESPACE

//! add shell index to leaves
/**
/param[in] apix sets the resolution of the surface
/note we assume that the leaves are xyz particles
 */
IMPMULTIFITEXPORT void add_surface_index(
    core::Hierarchy mhd, Float apix, FloatKey shell_key = FloatKey("surf_ind"),
    FloatKey radius_key = core::XYZR::get_radius_key(),
    FloatKey weight_key = atom::Mass::get_mass_key());

//! create a  weighted excluded volume restraint between two rigid bodies
/**
/param[in] rb1 the first rigid body
/param[in] rb2 the second rigid body
/param[in] shell_key the attribute that stored the particles
                     surface level with respect to its molecule
 */
IMPMULTIFITEXPORT IMP::Restraint* create_weighted_excluded_volume_restraint(
    core::RigidBody rb1, core::RigidBody rb2,
    FloatKey shell_key = FloatKey("surf_ind"));

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H */
