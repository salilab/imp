/**
 *  \file  m_rigid_fitting.h
 *  \brief preforms rigid fitting between a set of particles and a density map
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#ifndef IMPMULTIFIT_M_RIGID_FITTING_H
#define IMPMULTIFIT_M_RIGID_FITTING_H

#include <IMP/core/XYZR.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/VersionInfo.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/rigid_fitting.h>
#include "multifit_config.h"
#include <IMP/atom/Mass.h>
#include <IMP/ScoreState.h>
#include <IMP/container/ListSingletonContainer.h>
#include <algorithm>
IMPMULTIFIT_BEGIN_NAMESPACE

//! Compute fitting scores for a given set of rigid transformations
/**
\brief Fit a protein to its density by principle component matching
\param[in] ps       The particles to be fitted
\param[in] em_map   The density map to fit to
\param[in] rad_key  The raidus key of the particles in the rigid body
\param[in] wei_key  The weight key of the particles in the rigid body
\param[in] refine_fit if true local refinement by MC/CG is applied
\return fitting solutions
\note the function assumes the density map holds its density
*/
IMPMULTIFITEXPORT em::FittingSolutions pca_based_rigid_fitting(
  container::ListSingletonContainer *ps,
  em::DensityMap *em_map,Float threshold,
  bool refine_fit=false,
  FloatKey rad_key=core::XYZR::get_default_radius_key(),
  FloatKey wei_key=atom::Mass::get_mass_key());

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_M_RIGID_FITTING_H */
