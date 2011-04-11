/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/internal/inference_utility.h>
#include <IMP/domino/assignment_tables.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/progress.hpp>


IMPDOMINO_BEGIN_INTERNAL_NAMESPACE


namespace {

  std::pair<Subset, NodeData>
  get_best_conformations_internal(const MergeTree &jt,
                                  unsigned int root,
                                  const Subset& all,
                                  const SubsetFilterTables &filters,
                                  const AssignmentsTable *states,
                                  ListSubsetFilterTable *lsft,
                                  InferenceStatistics &stats,
                                  unsigned int max,
                                  boost::progress_display *progress) {
    typedef boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      SubsetMap;
    typedef boost::graph_traits<MergeTree>::adjacency_iterator
      NeighborIterator;

    SubsetMap subset_map= boost::get(boost::vertex_name, jt);
    IMP_FUNCTION_LOG;
    std::pair<NeighborIterator, NeighborIterator> be
      = boost::adjacent_vertices(root, jt);
    if (std::distance(be.first, be.second)==0) {
      // leaf
      std::pair<Subset, NodeData> ret;
      ret.first= boost::get(subset_map, root);
      IMP_LOG(VERBOSE, "Looking at leaf " << ret.first << std::endl);
      ret.second= get_node_data(ret.first, states);
      if (lsft) update_list_subset_filter_table(lsft, ret.first,
                                                ret.second.assignments);
      IMP_LOG(VERBOSE, "Subset data is\n" << ret.second << std::endl);
      stats.add_subset(ret.first, ret.second.assignments);
      return ret;
    } else {
      // merge
      IMP_INTERNAL_CHECK(std::distance(be.first, be.second)==2,
                         "Not a binary tree");
      std::pair<Subset, NodeData> cpd0
        = get_best_conformations_internal(jt, *be.first, all,
                                          filters,
                                          states, lsft,
                                          stats, max, progress);
      ++be.first;
      std::pair<Subset, NodeData> cpd1
        = get_best_conformations_internal(jt, *be.first, all,
                                          filters,
                                          states, lsft,
                                          stats, max, progress);
      EdgeData ed= get_edge_data(cpd0.first, cpd1.first, filters);
      std::pair<Subset, NodeData> ret;
      ret.first= ed.union_subset;
      ret.second= get_union(cpd0.first, cpd1.first, cpd0.second, cpd1.second,
                            ed, max);
      stats.add_subset(ed.union_subset, ret.second.assignments);
      if (lsft) update_list_subset_filter_table(lsft, ed.union_subset,
                                                ret.second.assignments);
      IMP_LOG(VERBOSE, "After merge, set is " << ret.first
              << " and data is\n" << ret.second << std::endl);
      if (progress) {
        ++(*progress);
      }
      return ret;
    }
  }
}

Assignments get_best_conformations(const MergeTree &mt,
                                    int root,
                                    const Subset& all_particles,
                                    const SubsetFilterTables &filters,
                                    const AssignmentsTable *states,
                                    ListSubsetFilterTable *lsft,
                                    InferenceStatistics &stats,
                                    unsigned int max) {
  boost::scoped_ptr<boost::progress_display> progress;
  if (get_log_level() == PROGRESS) {
    progress.reset(new boost::progress_display(boost::num_vertices(mt)));
  }
  return get_best_conformations_internal(mt, root,
                                         all_particles,
                                         filters,
                                         states, lsft,
                                         stats, max,
                                         progress.get()).second.assignments;
}


IMPDOMINO_END_INTERNAL_NAMESPACE
