/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/internal/inference_utility.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/pending/disjoint_sets.hpp>


/** This should be sped up by tracking which neighbor states are used
    to keep alive each state in a node and then removing the
    node when these lists are empty. This would avoid rechecking
    filters and make it purely a matter of pushing ints around.
*/

IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE
namespace {
void fill_node_data(const SubsetGraph &sg,
                    const SubsetStatesTable *sst,
                    std::vector<NodeData> &data) {
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
    subset_map= boost::get(boost::vertex_name, sg);
  data.resize(boost::num_vertices(sg));
  for (unsigned int i=0; i< data.size(); ++i) {
    data[i]= get_node_data(subset_map[i], sst);
  }
}
void fill_edge_data(const SubsetGraph &sg,
                    const SubsetFilterTables &sft,
                    std::vector<EdgeDatas> &data) {
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
  data.resize(boost::num_vertices(sg));
  for (unsigned int i=0; i< data.size(); ++i) {
    for (std::pair<NeighborIterator, NeighborIterator> be
           = boost::adjacent_vertices(i, sg);
         be.first != be.second; ++be.first) {
      if (*be.first < i) {
        data[i].push_back(get_edge_data(subset_map[i],
                                        subset_map[*be.first],
                                        sft));
      }
    }
  }
}

  bool loopy_filter_edge(const Subset &s0,
                        const Subset &s1,
                        NodeData &nd0,
                        const NodeData &nd1,
                        const EdgeData &ed) {
    Ints ii0= get_index(s0, ed.intersection_subset);
    Ints ii1= get_index(s1, ed.intersection_subset);
    Ints ui0= get_index(ed.union_subset, s0);
    Ints ui1= get_index(ed.union_subset, s1);
    Ints uii= get_index(ed.union_subset, ed.intersection_subset);
    Ints to_erase;
    for (unsigned int i=0; i< nd0.subset_states.size(); ++i) {
      bool ok=false;
      for (unsigned int j=0; j< nd1.subset_states.size(); ++j) {
        if (get_are_equal(nd0.subset_states[i], ii0,
                          nd1.subset_states[j], ii1)) {
          SubsetState ss= get_merged_subset_state(ed.union_subset,
                                                  nd0.subset_states[i], ui0,
                                                  nd1.subset_states[j], ui1);
          bool filter_ok=true;
          for (unsigned int i=0; i< ed.filters.size(); ++i) {
            if (ed.filters[i]->get_is_ok(ss)) {
            } else {
              filter_ok=false;
              break;
            }
          }
          if (filter_ok) {
            ok=true;
            break;
          }
        }
      }
      if (!ok) {
        to_erase.push_back(i);
      }
    }
    for (int i=to_erase.size()-1; i>=0; --i) {
      IMP_LOG(VERBOSE, "Erasing " << nd0.subset_states[to_erase[i]]
              << " from " << s0 << std::endl);
      nd0.subset_states.erase(nd0.subset_states.begin()+to_erase[i]);
    }
    return !to_erase.empty();
  }

bool filter_pass(const SubsetGraph &sg,
                 const std::vector<EdgeDatas> &eds,
                 std::vector<NodeData> &data) {
  bool changed=false;
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
  for (unsigned int i=0; i< data.size(); ++i) {
    int j=0;
    for (std::pair<NeighborIterator, NeighborIterator> be
           = boost::adjacent_vertices(i, sg);
         be.first != be.second; ++be.first) {
      if (*be.first < i) {
        changed |= loopy_filter_edge(subset_map[i],
                                     subset_map[*be.first],
                                     data[i], data[*be.first],
                                     eds[i][j]);
        changed |= loopy_filter_edge(subset_map[*be.first],
                                     subset_map[i],
                                     data[*be.first], data[i],
                                     eds[i][j]);
        ++j;
      }
    }
  }
  return changed;
}


}
SubsetStatesList loopy_get_best_conformations(const SubsetGraph &sg,
                                          const Subset& all_particles,
                                          const SubsetFilterTables &filters,
                                          const SubsetStatesTable *sst) {
  IMP_USAGE_CHECK(boost::num_vertices(sg) >0, "Must have a non-empty graph");
  std::vector<NodeData> nds;
  fill_node_data(sg, sst, nds);
  std::vector<EdgeDatas> eds;
  fill_edge_data(sg, filters, eds);
  boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, sg);
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Before filtering states are:\n");
    for (unsigned int i=0; i< nds.size(); ++i) {
      IMP_LOG(TERSE, subset_map[i] << ": ");
      for (unsigned int j=0; j< nds[i].subset_states.size(); ++j) {
        IMP_LOG(TERSE, nds[i].subset_states[j] << " ");
      }
      IMP_LOG(TERSE, std::endl);
    }
  }
  while (filter_pass(sg, eds, nds)) {
    IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Now states are:\n");
    for (unsigned int i=0; i< nds.size(); ++i) {
      IMP_LOG(VERBOSE, subset_map[i] << ": ");
      for (unsigned int j=0; j< nds[i].subset_states.size(); ++j) {
        IMP_LOG(VERBOSE, nds[i].subset_states[j] << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
  }
  }
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "After filtering states are:\n");
    for (unsigned int i=0; i< nds.size(); ++i) {
      IMP_LOG(TERSE, subset_map[i] << ": ");
      for (unsigned int j=0; j< nds[i].subset_states.size(); ++j) {
        IMP_LOG(TERSE, nds[i].subset_states[j] << " ");
      }
      IMP_LOG(TERSE, std::endl);
    }
  }
  NodeData curd=nds[0];
  Subset s= subset_map[0];
  for (unsigned int i=1; i< nds.size(); ++i) {
    Subset si= subset_map[i];
    EdgeData ed= get_edge_data(s, si, filters);
    curd= get_union(s, si, curd, nds[i], ed);
    s= ed.union_subset;
  }
  return curd.subset_states;
}

IMPDOMINO2_END_INTERNAL_NAMESPACE
