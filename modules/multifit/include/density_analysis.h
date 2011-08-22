/**
 *  \file density_analysis.h
 *  \brief density analysis tools, such as histogram and segmentation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_DENSITY_ANALYSIS_H
#define IMPMULTIFIT_DENSITY_ANALYSIS_H

#include <IMP/statistics/Histogram.h>
#include <IMP/em/DensityMap.h>
#include "multifit_config.h"
#include <IMP/domino/DiscreteSampler.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/graph/graph_utility.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Get a histogram of density values
IMPMULTIFITEXPORT statistics::Histogram get_density_histogram(
                   const em::DensityMap *dmap, float threshold,int num_bins);


typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::property<boost::vertex_index_t, long>,
                              boost::property<boost::edge_centrality_t, double
                                               > > DensityGraph;
typedef boost::graph_traits<DensityGraph>::vertex_descriptor DGVertex;
typedef boost::graph_traits<DensityGraph>::edge_descriptor DGEdge;

class IMPMULTIFITEXPORT DensitySegmentationByCommunities {
public:
  //! Constructor
  /**
     \param[input] dmap the map to segment
     \param[input] dens_t only consider voxels above this threshold
   */
  DensitySegmentationByCommunities(em::DensityMap *dmap,
                                   float dens_t):
    dmap_(dmap),dens_t_(dens_t){}
  //! Build the density graph
  /**
     \param[input] edge_threshold two voxels are considered
     neighbors if the difference between their density does
     not exceed this number
   */
  void build_density_graph(float neighbor_threshold);
  //! Graph clustering based on edge betweenness centrality.
  //! It is an iterative algorithm, where in each step it computes
  //! the edge betweenness centrality (via brandes_betweenness_centrality)
  //! and removes the edge with the maximum betweenness centrality.
  domino::IntsList calculate_communities(int num_clusters);
  algebra::Vector3Ds get_centers() {
    algebra::Vector3Ds centers;
    DGVertex v;
    for (std::pair<boost::graph_traits<DensityGraph>::vertex_iterator,
                   boost::graph_traits<DensityGraph>::vertex_iterator> be=
           boost::vertices(g_);
         be.first != be.second; ++be.first) {
      v=*be.first;
      centers.push_back(
          dmap_->get_location_by_voxel(node2voxel_ind_[node_index_[v]]));
    }
    return centers;
  }
  domino::IntsList calculate_connected_components();
 protected:
  Pointer<em::DensityMap> dmap_;
  DensityGraph g_;
  float dens_t_;
  std::vector<float> weights_;
  std::vector<long> node2voxel_ind_;
  boost::property_map<DensityGraph, boost::vertex_index_t>::type node_index_;
  typedef boost::disjoint_sets<int*, int*> DS;
  /*The function object that indicates termination of the algorithm.
    It must be a ternary function object thats accepts the maximum centrality,
    the descriptor of the edge that will be removed, and the graph g.
  */
  struct Done {
    typedef double centrality_type;
    int k_;
    std::vector<int> rank_, parent_;
    //rank stores the size of the set, and parent the parent
    /**
       \pram[in] k the maximum number of clsuters
       \param[in] n the number of elements
    */
    Done(int k, int n): k_(k), rank_(n), parent_(n){}
    template <class B>
    /**
       \param[input] c the current maximum centrality
       \param[input] e the edge that is being examined
       \param[input] g the graph
       \note The function calculates the new conneted components
             of the system after the edge was removed
       \note We will be removing edges until we have more than k components
    */
    bool operator()(centrality_type c, const B & e, const DensityGraph &g) {
      std::cout << "Done called on " << boost::source(e, g)
                << "--" << boost::target(e, g)
                << ": " << c << std::endl;
      DS ds(&rank_[0], &parent_[0]);
      //Basic initialization of the disjoint-sets structure.
      //Each vertex in the graph g is in its own set.
      boost::initialize_incremental_components(g, ds);
      //The connected components are calculated based on
      //the edges in the graph g and the information is embedded in ds.
      boost::incremental_components(g, ds);
      unsigned int s= ds.count_sets(boost::vertices(g).first,
                                    boost::vertices(g).second);
      //if there are more than k components we are done
      std::cout<<"s:"<<s<<"k:"<<k_<<std::endl;
      return s >= static_cast<unsigned int>(k_);
    }
  };
};


IMPMULTIFITEXPORT em::DensityMaps density_segmentation(
         em::DensityMap *dmap,
         float dens_t,float edge_threshold,
         int num_clusters);

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
domino::IntsList get_connected_components(
                                          em::DensityMap *dmap,
                                          float threshold,float edge_threshold);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DENSITY_ANALYSIS_H */
