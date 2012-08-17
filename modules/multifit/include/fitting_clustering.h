/**
 *  \file IMP/multifit/fitting_clustering.h
 *  \brief Cluster fitting solutions by rmsd
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_FITTING_CLUSTERING_H
#define IMPMULTIFIT_FITTING_CLUSTERING_H

#include <string>
#include <vector>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/em/rigid_fitting.h>
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE

/**
\param[in] mh cluster transformations with respect to this protein
\param[in] ts the transformations with respect to the transformation in the PDB
\param[in] spacing transformations with rmsd < spacing*2 are clustered together
\param[in] top_sols cluster no more than this many top solutions
\param[in] rmsd maximum RMSD between transformations in the same cluster
\return the clustered transformations
*/
em::FittingSolutions IMPMULTIFITEXPORT fitting_clustering (
    const atom::Hierarchy &mh,
    const em::FittingSolutions &ts,
    float spacing, int top_sols, float rmsd);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_FITTING_CLUSTERING_H */
