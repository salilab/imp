/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/subset_evaluators.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE
/*
struct EdgeData {
  Pointer<SubsetFilter> sft_;
  Int in_indexes_, out_indexes_;
};
typedef std::vector<EdgeData> EdgeDatas;

void loopy_compute_initial_sets(const SubsetGraph &sg,
                                SubsetStatesTable *sst,
                                std::vector<SubsetStates> &data);
void loopy_compute_edge_data(const SubsetGraph &sg,
                           SubsetFiltersTable *sft,
                           std::vector<EdgeDatas> &sfts);

bool loopy_filter_one(SubsetData &sd,
                      const EdgeData &sft,
                      const SubsetData &nd) {

}

bool loopy_filter_sets(const SubsetGraph &sg,
                       const std::vector<EdgeDatas> &sfts,
                       std::vector<SubsetStates> &data) {
  bool changed=false;
  for (unsigned int i=0; i< data.size(); ++i) {
    for (unsigned int j=0; j< boost::num_out_edges(i, sg); ++j) {
      int n=boost::target(boost::out_edge(i, j, sg));
      changed |= loopy_filter_one(data[i], data[n], sfts[i][j]);
    }
  }
  return changed;
}

SubsetStates loopy_get_best_solutions(const SubsetGraph &sg,
                                        const std::vector<SubsetStates> &data,
                                        const std::vector<EdgeDatas> &edatas) {
  IncompleteStates states;
  for (unsigned int i=0; i< data.size(); ++i) {
    expand_states(states, data[i], vm[i]);
  }
  return get_subset_states(states);
  }*/

IMPDOMINO2_END_INTERNAL_NAMESPACE
