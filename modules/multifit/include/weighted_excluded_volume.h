/**
 *  \file weighted_excluded_volume.h
 *  \brief handles low resolution weighted excluded
 *         volume calculation.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H
#define IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/atom/Mass.h>
#include "config.h"
IMPMULTIFIT_BEGIN_NAMESPACE


//! add shell index to leaves
/**
/param[in] apix sets the resolution of the surface
/note we assume that the leaves are xyz particles
 */
IMPMULTIFITEXPORT void add_surface_index(core::Hierarchy mhd,
                Float resolution, Float apix,
                FloatKey shell_key= FloatKey("surf_ind"),
                FloatKey radius_key=core::XYZR::get_default_radius_key(),
                FloatKey weight_key=atom::Mass::get_mass_key()) ;


IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_WEIGHTED_EXCLUDED_VOLUME_H */
