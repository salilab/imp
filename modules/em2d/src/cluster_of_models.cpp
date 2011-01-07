/**
 *  \file cluster_of_models.cpp
 *  \brief Cluster of models scored with em2d using the RMSD between them
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include  "IMP/em2d/cluster_of_models.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/model_interaction.h"
#include "IMP/algebra/Transformation3D.h"
#include "IMP/algebra/geometric_alignment.h"
#include "IMP/multifit/RMSDClustering.h"
#include "IMP/core/XYZ.h"
#include "IMP/Pointer.h"
#include <algorithm>

IMPEM2D_BEGIN_NAMESPACE

TransformationsClusters
      cluster_models_by_em2d(const String &selection_file,
                            const Floats &em2d_scores,
                            double rmsd_cutoff,
                            double ratio_bin_size_rmsd) {
  IMP_NEW(IMP::Model,model,());
  IMP::Pointer<atom::ATOMPDBSelector> selector=new atom::ATOMPDBSelector();
  bool select_first_model=true;
  atom::Hierarchies mhs=read_multiple_pdb_files(selection_file,
                                  model,
                                  selector,
                                  select_first_model);
  return cluster_models_by_em2d(mhs,
                                em2d_scores,
                                rmsd_cutoff,
                                ratio_bin_size_rmsd);
}

TransformationsClusters
       cluster_models_by_em2d(const atom::Hierarchies &mhs,
                              const Floats &em2d_scores,
                              double rmsd_cutoff,
                              double ratio_bin_size_rmsd) {
  IMP_LOG(TERSE,"Clustering models...");
  IMP_USAGE_CHECK(mhs.size()==em2d_scores.size(),
  "cluster_models_by_em2d: Hierarchies and scores don't have the same size");

  // get the best model by em2d score
  Floats::const_iterator it=std::min_element(em2d_scores.begin(),
                                             em2d_scores.end());
  unsigned int  min_em2d_index = it-em2d_scores.begin();
  IMP_LOG(TERSE,"Best model: index = " << min_em2d_index << " em2d score = "
          << *it << std::endl);

  ParticlesTemp best_model_ps = core::get_leaves(mhs[min_em2d_index]);
  core::XYZsTemp best_model_xyzs(best_model_ps.begin(),best_model_ps.end());

  // get the transformations3D of all models respect to the best.
  TransformationsClusters clusters;
  for (unsigned int i=0;i<mhs.size();++i) {
    if(i != min_em2d_index) {
      ParticlesTemp other_model_ps= core::get_leaves(mhs[i]);
      core::XYZsTemp other_model_xyzs(other_model_ps.begin(),
                                      other_model_ps.end());
      // get the transformation based on minimizing the RMSD
      algebra::Transformation3D transformation=
         algebra::get_transformation_aligning_first_to_second(other_model_xyzs,
                                                      best_model_xyzs);
      // use the ccc instead of the em2d score. The clustering requires
      // that the higher the score, the better.
      TransformationsCluster cluster(transformation,
                                     get_em2d_to_ccc(em2d_scores[i]));
      clusters.push_back(cluster);
    }
  }
  TransformationsClusters clustered;
  multifit::RMSDClustering<TransformationsCluster> clusterer;
  clusterer.prepare(best_model_ps);
  clusterer.set_bin_size(ratio_bin_size_rmsd*rmsd_cutoff);
  clusterer.cluster(rmsd_cutoff,clusters,clustered);
  IMP_LOG(TERSE,"Clusters obtained" << clustered.size() << std::endl);
  return clustered;
}

IMPEM2D_END_NAMESPACE
