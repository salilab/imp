/**
 *  \file IMP/multifit/pca_based_rigid_fitting.h
 *  \brief preforms rigid fitting between a set of particles and a density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMULTIFIT_PCA_BASED_RIGID_FITTING_H
#define IMPMULTIFIT_PCA_BASED_RIGID_FITTING_H

#include <IMP/core/XYZR.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/rigid_fitting.h>
#include <IMP/multifit/multifit_config.h>
#include <IMP/atom/Mass.h>
#include <IMP/ScoreState.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/algebra/eigen_analysis.h>
#include <algorithm>
IMPMULTIFIT_BEGIN_NAMESPACE


//! Compute fitting scores for a given set of rigid transformations
/**
\brief Fit a protein to its density by principal component matching
\param[in] rb         The rigid body to be fitted
\param[in] rb_refiner The rigid body refiner
\param[in] em_map     The density map to fit to
\param[in] threshold  Use voxels above this threshold for PCA calculations
\param[in] wei_key  The weight key of the particles in the rigid body
\param[in] dens_pca_input provide precalculated em_map PCA is available
\return fitting solutions
\note the function assumes the density map holds its density
*/
IMPMULTIFITEXPORT em::FittingSolutions pca_based_rigid_fitting(
  core::RigidBody rb, Refiner *rb_refiner,
  em::DensityMap *em_map,Float threshold,
  FloatKey wei_key=atom::Mass::get_mass_key(),
  algebra::PrincipalComponentAnalysis dens_pca_input=
   algebra::PrincipalComponentAnalysis());



//! Compute fitting scores for a given set of rigid transformations
/**
\brief Fit a protein to its density by principal component matching
\param[in] ps         The particles to fit (treated as rigid)
\param[in] em_map     The density map to fit to
\param[in] threshold  Use voxels above this threshold for PCA calculations
\param[in] wei_key  The weight key of the particles in the rigid body
\param[in] dens_pca_input precalculated em_map PCA
\return fitting solutions
\note the function assumes the density map holds its density
*/
IMPMULTIFITEXPORT em::FittingSolutions pca_based_rigid_fitting(
  kernel::ParticlesTemp ps,
  em::DensityMap *em_map,Float threshold,
  FloatKey wei_key=atom::Mass::get_mass_key(),
  algebra::PrincipalComponentAnalysis dens_pca_input=
   algebra::PrincipalComponentAnalysis());

IMPMULTIFITEXPORT
void write_markers(const algebra::PrincipalComponentAnalysisD<3> &pca,
                   std::ostream &out);

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_PCA_BASED_RIGID_FITTING_H */
