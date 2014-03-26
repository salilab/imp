/**
 *  \file IMP/multifit/density_analysis.h
 *  \brief density analysis tools, such as segmentation
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DENSITY_ANALYSIS_H
#define IMPMULTIFIT_DENSITY_ANALYSIS_H

#include <IMP/em/DensityMap.h>
#include <IMP/multifit/multifit_config.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/graph/graph_utility.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Returns a map containing all density without the background
/**
\param[in] dmap the density map to segment
\param[in] threshold consider only voxels above this threshold
\param[in] edge_threshold consider only voxels above this threshold
\return the segmented map
 */
IMPMULTIFITEXPORT em::DensityMap *remove_background(em::DensityMap *dmap,
                                                    float threshold,
                                                    float edge_threshold);

//! Return connected components based on density values
/**
   \param[in] dmap the density map to analyze
   \param[in] threshold consider only voxels above this threshold
   \param[in] edge_threshold an edge is added between
     two neighboring voxels if their density difference is below this threshold
   \return List of indexes for each connected component
*/
IMPMULTIFITEXPORT
IntsList get_connected_components(em::DensityMap *dmap, float threshold,
                                  float edge_threshold);

//! Segment a density map using the anchor graph.
/** All voxels above the threshold are segmented into the given number
    of clusters, and neighboring clusters are linked.

    \param[in] dmap the density map to segment
    \param[in] apix the map spacing in angstroms per pixel
    \param[in] density_threshold consider only voxels over this threshold
    \param[in] num_means the number of segments to generate
    \param[in] pdb_filename write cluster centers as CA atoms in PDB format
    \param[in] cmm_filename if not empty, write clusters in CMM format
    \param[in] seg_filename if not empty, write segments in MRC format
    \param[in] txt_filename if not empty, write anchors in text format
 */
IMPMULTIFITEXPORT
void get_segmentation(em::DensityMap *dmap, double apix,
                      double density_threshold, int num_means,
                      const std::string pdb_filename,
                      const std::string cmm_filename,
                      const std::string seg_filename,
                      const std::string txt_filename);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DENSITY_ANALYSIS_H */
