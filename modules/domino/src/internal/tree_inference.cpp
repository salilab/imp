/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/internal/inference_utility.h>
#include <IMP/domino/subset_states.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO_BEGIN_INTERNAL_NAMESPACE


namespace {

  std::pair<Subset, NodeData>
  get_best_conformations_internal(const SubsetGraph &jt,
                                  unsigned int root,
                                  unsigned int parent,
                                  const Subset& all,
                                  const SubsetFilterTables &filters,
                                  const SubsetStatesTable *states,
                                  ListSubsetFilterTable *lsft,
                                  InferenceStatistics &stats) {
    SubsetMap subset_map= boost::get(boost::vertex_name, jt);
    Subset s;
    IncreaseIndent ii;
    bool initialized=false;
    NodeData nd;
    for (std::pair<NeighborIterator, NeighborIterator> be
           = boost::adjacent_vertices(root, jt);
         be.first != be.second; ++be.first) {
      if (*be.first == parent) continue;
      // compute intersection set and index map in one direction
      // for each pattern of that in me, compute subset score
      // subtract the min of mine (assume scores positive)
      // for merged score, subtract off edge value
      std::pair<Subset, NodeData> cpd
        = get_best_conformations_internal(jt, *be.first, root, all,
                                          filters,
                                          states, lsft,
                                          stats);
      if (!initialized) {
        IMP_LOG(VERBOSE, "Looking at subset " << s << std::endl);
        s= boost::get(subset_map, root);
        nd= get_node_data(s, states);
        if (lsft) update_list_subset_filter_table(lsft, s, nd.subset_states);
        IMP_LOG(VERBOSE, "Subset data is\n" << nd << std::endl);
        initialized=true;
        stats.add_graph_subset(s, nd.subset_states);
      }
      EdgeData ed= get_edge_data(s, cpd.first, filters);
      nd= get_union(s, cpd.first, nd, cpd.second, ed);
      stats.add_merged_subset(ed.union_subset, nd.subset_states);
      s= ed.union_subset;
      if (lsft) update_list_subset_filter_table(lsft, s, nd.subset_states);
      IMP_LOG(VERBOSE, "After merge, set is " << s
              << " and data is\n" << nd << std::endl);
    }
    if (!initialized) {
        IMP_LOG(VERBOSE, "Looking at subset " << s << std::endl);
        s= boost::get(subset_map, root);
        nd= get_node_data(s, states);
        if (lsft) update_list_subset_filter_table(lsft, s, nd.subset_states);
        IMP_LOG(VERBOSE, "Subset data is\n" << nd << std::endl);
        initialized=true;
        stats.add_graph_subset(s, nd.subset_states);
      }
    return std::make_pair(s, nd);
  }
}

SubsetStates get_best_conformations(const SubsetGraph &jt,
                                    int root,
                                    const Subset& all_particles,
                                    const SubsetFilterTables &filters,
                                    const SubsetStatesTable *states,
                                    ListSubsetFilterTable *lsft,
                                    InferenceStatistics &stats) {
  return get_best_conformations_internal(jt, root, root,
                                         all_particles,
                                         filters,
                                         states, lsft,
                                         stats).second.subset_states;
}


IMPDOMINO_END_INTERNAL_NAMESPACE
