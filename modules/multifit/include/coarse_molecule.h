/**
 *  \file IMP/multifit/coarse_molecule.h
 *  \brief coarsen molecule by clustering
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_COARSE_MOLECULE_H
#define IMPMULTIFIT_COARSE_MOLECULE_H

#include <IMP/atom/Hierarchy.h>
#include <IMP/em/DensityMap.h>
#include <IMP/multifit/multifit_config.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Coarsen a molecule based on atom clustering
/**
\param[in] mh the molecule to coarsen
\param[in] num_beads the output number of beads
\param[in] mdl model to add the new molecule to
\param[in] bead_radius bead radius
\param[in] add_conn_restraint if true a connectivity restraint
             between the particles of the new molecule is added
 */
IMPMULTIFITEXPORT
atom::Hierarchy create_coarse_molecule_from_molecule(
    const atom::Hierarchy &mh, int num_beads, kernel::Model *mdl,
    float bead_radius, bool add_conn_restraint = false);

//! Coarsen molecules based on atom clustering
/**
\param[in] mhs the molecules to coarsen
\param[in] frag_len the number of beads for each molecule will be
                    its number of residues / frag_len
\param[in] mdl model to add the new molecule to
\param[in] bead_radius bead radius
\param[in] add_conn_restraint if true a connectivity restraint
           between the particles of the new molecule is added
 */
IMPMULTIFITEXPORT
atom::Hierarchies create_coarse_molecules_from_molecules(
    const atom::Hierarchies &mhs, int frag_len, kernel::Model *mdl,
    float bead_radius, bool add_conn_restraint = false);

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
                                                    kernel::Model *mdl,
                                                    float bead_radius);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_COARSE_MOLECULE_H */
