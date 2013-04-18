/**
 *  \file IMP/multifit/density_analysis.h
 *  \brief density analysis tools, such as histogram and segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DENSITY_ANALYSIS_H
#define IMPMULTIFIT_DENSITY_ANALYSIS_H

#include <IMP/statistics/Histogram.h>
#include <IMP/em/DensityMap.h>
#include <IMP/multifit/multifit_config.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/graph/graph_utility.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Get a histogram of density values
IMPMULTIFITEXPORT statistics::Histogram get_density_histogram(
                   const em::DensityMap *dmap, float threshold,int num_bins);

//! Returns a map containing all density without the background
/**
\param[in] dmap the density map to segment
\param[in] threshold consider only voxels above this threshold
\param[in] edge_threshold consider only voxels above this threshold
\return the segmented map
 */
IMPMULTIFITEXPORT em::DensityMap* remove_background(em::DensityMap *dmap,
                                      float threshold,float edge_threshold);


//! Return connected components based on density values
/**
   \param[in] dmap the density map to analyze
   \param[in] threshold consider only voxels above this threshold
   \param[in] edge_threshold an edge is added between
     two neighboring voxels if their density difference is below this threshold
   \return List of indexes for each connected component
*/
IMPMULTIFITEXPORT
IntsList get_connected_components(
                                          em::DensityMap *dmap,
                                          float threshold,float edge_threshold);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DENSITY_ANALYSIS_H */
