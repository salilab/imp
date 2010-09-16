/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/internal/inference_utility.h>
#include <IMP/domino2/subset_states.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE


namespace {
  bool get_are_equal(const SubsetState &ss0,
                     const Ints &i0,
                     const SubsetState &ss1,
                     const Ints &i1) {
    IMP_USAGE_CHECK(i0.size() == i1.size(), "Sizes don't match");
    for (unsigned int i=0; i< i0.size(); ++i) {
      if (ss0[i0[i]] != ss1[i1[i]]) return false;
    }
    return true;
  }

  SubsetState get_merged_subset_state(const Subset &s,
                                     const SubsetState &ss0,
                                     const Ints &i0,
                                     const SubsetState &ss1,
                                     const Ints &i1) {
    Ints ret(s.size(), -1);
    IMP_USAGE_CHECK(ss0.size() == i0.size(), "Don't match");
    IMP_USAGE_CHECK(ss1.size() == i1.size(), "Don't match");
    for (unsigned int i=0; i < i0.size(); ++i) {
      ret[i0[i]]= ss0[i];
    }
    for (unsigned int i=0; i < i1.size(); ++i) {
      ret[i1[i]]= ss1[i];
    }
    IMP_IF_CHECK(USAGE) {
      for (unsigned int i=0; i< ret.size(); ++i) {
        IMP_USAGE_CHECK(ret[i] >=0, "Not all set");
      }
    }
    return SubsetState(ret);
  }

  NodeData
  get_union(const Subset &s0, const Subset &s1,
            const NodeData &nd0, const NodeData &nd1,
            const EdgeData &ed) {
    NodeData ret;
    Ints ii0= get_index(s0, ed.intersection_subset);
    Ints ii1= get_index(s1, ed.intersection_subset);
    Ints ui0= get_index(ed.union_subset, s0);
    Ints ui1= get_index(ed.union_subset, s1);
    Ints uii= get_index(ed.union_subset, ed.intersection_subset);
    for (unsigned int i=0; i< nd0.subset_states.size(); ++i) {
      for (unsigned int j=0; j< nd1.subset_states.size(); ++j) {
        if (get_are_equal(nd0.subset_states[i], ii0,
                          nd1.subset_states[j], ii1)) {
          SubsetState ss= get_merged_subset_state(ed.union_subset,
                                                  nd0.subset_states[i], ui0,
                                                  nd1.subset_states[j], ui1);
          bool ok=true;
          for (unsigned int i=0; i< ed.filters.size(); ++i) {
            if (ed.filters[i]->get_is_ok(ss)) {
              // pass
            } else {
              ok=false;
              break;
            }
          }
          if (ok) {
            ret.subset_states.push_back(ss);
          }
        }
      }
    }
    return ret;
  }


  std::pair<Subset, NodeData>
  get_best_conformations_internal(const SubsetGraph &jt,
                                  unsigned int root,
                                  unsigned int parent,
                                  const Subset& all,
                                  const SubsetFilterTables &filters,
                                  const SubsetStatesTable *states) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, jt);
    Subset s= boost::get(subset_map, root);
    IMP_LOG(VERBOSE, "Looking at subset " << s << std::endl);
    IncreaseIndent ii;
    NodeData nd= get_node_data(s, states);
    IMP_LOG(VERBOSE, "Subset data is\n" << nd << std::endl);
    typedef boost::graph_traits<SubsetGraph>::adjacency_iterator
      NeighborIterator;
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
                                          states);
      EdgeData ed= get_edge_data(s, cpd.first, filters);
      nd= get_union(s, cpd.first, nd, cpd.second, ed);
      s= ed.union_subset;
      IMP_LOG(VERBOSE, "After merge, set is " << s
              << " and data is\n" << nd << std::endl);
    }
    return std::make_pair(s, nd);
  }
}

SubsetStatesList get_best_conformations(const SubsetGraph &jt,
                                      int root,
                                      const Subset& all_particles,
                                      const SubsetFilterTables &filters,
                                      const SubsetStatesTable *states) {
  return get_best_conformations_internal(jt, root, root,
                                         all_particles,
                                         filters,
                                         states).second.subset_states;
}


IMPDOMINO2_END_INTERNAL_NAMESPACE
