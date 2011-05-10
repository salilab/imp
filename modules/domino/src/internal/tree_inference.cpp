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
Assignments get_merged_assignments(const Subset &first_subset,
                                   const Assignments &first,
                                   const Subset &second_subset,
                                   const Assignments &second,
                                   const SubsetFilterTables &filters,
                                   ListSubsetFilterTable *lsft,
                                   InferenceStatistics &stats,
                                   unsigned int max) {
  IMP_FUNCTION_LOG;
  EdgeData ed= get_edge_data(first_subset, second_subset, filters);
  Assignments ret;
  ret= get_union(first_subset, second_subset, first, second,
                 ed, max);
  stats.add_subset(ed.union_subset, ret);
  if (lsft) update_list_subset_filter_table(lsft, ed.union_subset,
                                            ret);
  /*using namespace IMP;
  IMP_LOG(VERBOSE, "After merge, set is " << merged_subset
  << " and data is\n" << ret << std::endl);*/
  return ret;
}
Assignments get_leaf_assignments(const Subset &merged_subset,
                                     const AssignmentsTable *states,
                                     ListSubsetFilterTable *lsft,
                                     InferenceStatistics &stats) {
  IMP_FUNCTION_LOG;
  IMP_LOG(VERBOSE, "Looking at leaf " << merged_subset << std::endl);
  Assignments ret= get_node_data(merged_subset, states);
  if (lsft) update_list_subset_filter_table(lsft, merged_subset,
                                            ret);
  //using namespace IMP;
  //IMP_LOG(VERBOSE, "Subset data is\n" << ret << std::endl);
  stats.add_subset(merged_subset, ret);
  return ret;
}

namespace {

  Assignments
  get_best_conformations_internal(const MergeTree &jt,
                                  unsigned int root,
                                  const Subset& all,
                                  const AssignmentsTable *states,
                                  const SubsetFilterTables &filters,
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
      return get_leaf_assignments(boost::get(subset_map, root),
                                  states, lsft, stats);
    } else {
      // merge
      IMP_INTERNAL_CHECK(std::distance(be.first, be.second)==2,
                         "Not a binary tree");
      int firsti=*be.first;
      int secondi= *(++be.first);
      Assignments cpd0
        = get_best_conformations_internal(jt, firsti, all, states,
                                          filters, lsft,
                                          stats, max, progress);
      Assignments cpd1
        = get_best_conformations_internal(jt, secondi, all, states,
                                          filters, lsft,
                                          stats, max, progress);
      Assignments ret= get_merged_assignments(boost::get(subset_map, firsti),
                                              cpd0,
                                              boost::get(subset_map, secondi),
                                              cpd1,
                                              filters, lsft, stats, max);
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
                                         all_particles,states,
                                         filters,
                                         lsft,
                                         stats, max,
                                         progress.get());
}


IMPDOMINO_END_INTERNAL_NAMESPACE
