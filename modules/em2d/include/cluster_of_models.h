/**
 *  \file cluster_of_models.h
 *  \brief functions to cluster the models according to the em2d score
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_CLUSTER_OF_MODELS_H
#define IMPEM2D_CLUSTER_OF_MODELS_H

#include "IMP/em2d/TransformationsCluster.h"
#include "IMP/Model.h"
#include "IMP/atom/pdb.h"
#include "IMP/atom/Hierarchy.h"

IMPEM2D_BEGIN_NAMESPACE

// Clustering of models obtained from scoring with em2d
/*!
  \param[in] selection_file file with the names of the models. See
            read_selection_file()
  \note See the rest of the values in cluster_models_by_em2d()
*/
IMPEM2DEXPORT TransformationsClusters
 cluster_models_by_em2d(const String &selection_file,
                            const Floats &em2d_scores,
                            double rmsd_cutoff,
                            double ratio_bin_size_rmsd=0.5);

// Clustering of models obtained from scoring with em2d
/*!
  \param[in] mhs molecular hierarchies containing models (one hierarchy per
             model).
  \param[in] em2d_scores Scores for the models. The must have the saqme order
            as the models in the selection file
  \param[in] rmsd_cutoff RMSD value used as a cutoff for building clusters
  \param[in] ratio_bin_size_rmsd.
  \param[out] clusters of transformations of the model that are similar
  \note Models here mean the different models generated for a structure, not
        the Model object. All the hierarchies here are part of a Model, but
        each hierarchy is a model.
*/
IMPEM2DEXPORT TransformationsClusters
       cluster_models_by_em2d(const atom::Hierarchies &mhs,
                            const Floats &em2d_scores,
                            double rmsd_cutoff,
                            double ratio_bin_size_rmsd=0.5);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_CLUSTER_OF_MODELS_H */
