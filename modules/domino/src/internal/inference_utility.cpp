/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/internal/inference_utility.h>
#include <IMP/domino/subset_states.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO_BEGIN_INTERNAL_NAMESPACE


InferenceStatistics::InferenceStatistics(){}

void InferenceStatistics::write_data(const Data &data,
                                     std::ostream &out) const {
  out << data.subset << ": " << data.size << ": ";
  for (unsigned int i=0; i< data.sample.size(); ++i) {
    out << data.sample[i] << " ";
  }
  out << std::endl;
}

InferenceStatistics::Data
InferenceStatistics::get_data(const Subset &s, SubsetStates ss) const {
  Data ret;
  ret.subset=s;
  ret.size= ss.size();
  std::random_shuffle(ss.begin(), ss.end());
  ret.sample=SubsetStates(ss.begin(), ss.begin()
                          +std::min(ss.size(), size_t(10)));
  return ret;
}

void InferenceStatistics::add_graph_subset(const Subset &s,
                                           const SubsetStates &ss) {
  graph_subsets_.push_back(get_data(s, ss));
}

void InferenceStatistics::add_merged_subset(const Subset &s,
                                            const SubsetStates &ss) {
  merged_subsets_.push_back(get_data(s, ss));
}


InferenceStatistics::~InferenceStatistics() {
  for (unsigned int i=0; i< graph_subsets_.size(); ++i) {
    std::cerr << "Leaf ";
    write_data(graph_subsets_[i], std::cerr);
  }
  for (unsigned int i=0; i< graph_subsets_.size(); ++i) {
    std::cerr << "Merged ";
    write_data(graph_subsets_[i], std::cerr);
  }
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


IMPDOMINO_END_INTERNAL_NAMESPACE
