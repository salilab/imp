/**
 *  \file IMP/multifit/coarse_molecule.h
 *  \brief coarsen molecule by clustering
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_COARSE_MOLECULE_H
#define IMPMULTIFIT_COARSE_MOLECULE_H

#include <IMP/atom/Hierarchy.h>
#include <IMP/em/DensityMap.h>
#include <IMP/multifit/multifit_config.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Coarsen a density map based on voxels clustering
/**
\param[in] dmap the density map to coarsen
\param[in] dens_threshold use only voxels above this
                             threshold for clustering
\param[in] num_beads the number of beads
\param[in] mdl model to add the new molecule to
\param[in] bead_radius bead radius
 */
IMPMULTIFITEXPORT
atom::Hierarchy create_coarse_molecule_from_density(em::DensityMap *dmap,
                                                    float dens_threshold,
                                                    int num_beads,
                                                    Model *mdl,
                                                    float bead_radius);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_COARSE_MOLECULE_H */
