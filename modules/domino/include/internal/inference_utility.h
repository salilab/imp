/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_INFERENCE_UTILITY_H
#define IMPDOMINO_INTERNAL_INFERENCE_UTILITY_H

#include <IMP/domino/domino_config.h>
#include "../utility.h"
#include "../subset_filters.h"
#include "../assignment_tables.h"
#include "../subset_graphs.h"

#include <IMP/Model.h>
#include <IMP/ScoreState.h>
#include <IMP/Restraint.h>

#include <vector>
#include <IMP/base/map.h>
#include <sstream>

IMPDOMINO_BEGIN_NAMESPACE
class SubsetEvaluatorTable;
class AssignmentsTable;
IMPDOMINO_END_NAMESPACE

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

class IMPDOMINOEXPORT InferenceStatistics {
  struct Data {
    int size;
    Assignments sample;
  };
  Data get_data(const Subset &s, AssignmentContainer* ss) const;
  IMP::base::map<Subset, Data> subsets_;
  mutable boost::uniform_real<double> select_;
  mutable boost::uniform_int<> place_;
  const Data & get_data(const Subset &s) const;
public:
  InferenceStatistics();
  void add_subset(const Subset &s, AssignmentContainer *ss);
  unsigned int get_number_of_assignments(Subset subset) const;
  Assignments get_sample_assignments(Subset subset) const;
  ~InferenceStatistics();
};

struct AsIndexes {
  Ints is_;
  AsIndexes(Ints is): is_(is){}
  void show(std::ostream &out) const {
    for (unsigned int i=0; i< is_.size(); ++i) {
      if (is_[i]>=0) {
        out << is_[i] << " ";
      } else {
        out << "- ";
      }
    }
  }
};
IMP_VALUES(AsIndexes, AsIndexesList);





typedef IMP::base::map<Particle*, int> ParticleIndex;

inline ParticleIndex get_index(const Subset &s) {
  ParticleIndex ret;
  for (unsigned int i=0; i< s.size(); ++i) {
    ret[s[i]]=i;
  }
  return ret;
}


inline Ints get_index(const Subset &s, const Subset &subs) {
  Ints ret(subs.size(), -1);
  for (unsigned int i=0; i< subs.size(); ++i) {
    for (unsigned int j=0; j< s.size(); ++j) {
      if (s[j] == subs[i]) {
        ret[i]=j;
      }
    }
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ret.size(); ++i) {
      IMP_USAGE_CHECK(ret[i] >=0, "Second is not is not a subset of first.");
    }
  }
  return ret;
}


struct EdgeData {
  Subset intersection_subset;
  Subset union_subset;
  SubsetFilters filters;
};
typedef base::Vector<EdgeData> EdgeDatas;

inline std::ostream &operator<<(std::ostream &out, const EdgeData &nd) {
  out << nd.intersection_subset << " " << nd.union_subset << std::endl;
  return out;
}

inline EdgeData get_edge_data(const Subset&s0,
                              const Subset&s1,
                              const SubsetFilterTables &sft) {
  EdgeData ret;
  ret.union_subset= get_union(s0, s1);
  ret.intersection_subset= get_intersection(s0,s1);
  Subsets excluded;
  excluded.push_back(s0);
  excluded.push_back(s1);
  for (unsigned int i=0; i< sft.size(); ++i) {
    SubsetFilter* sf= sft[i]->get_subset_filter(ret.union_subset,
                                                excluded);
    if (sf) {
      sf->set_was_used(true);
      ret.filters.push_back(sf);
    }
  }
  return ret;
}


inline Assignment get_sub_assignment(const Assignment &ss0,
                                     const Ints &i0) {
  Ints ret(i0.size());
  for (unsigned int i=0; i< i0.size(); ++i) {
    ret[i]= ss0[i0[i]];
  }
  return Assignment(ret);
}

//! return true if the two states are equal on the entries in the lists
inline bool get_are_equal(const Assignment &ss0,
                          const Ints &i0,
                          const Assignment &ss1,
                          const Ints &i1) {
  IMP_USAGE_CHECK(i0.size() == i1.size(), "Sizes don't match");
  for (unsigned int i=0; i< i0.size(); ++i) {
    if (ss0[i0[i]] != ss1[i1[i]]) return false;
  }
  return true;
}


IMPDOMINOEXPORT
Assignment get_merged_assignment(const Subset &s,
                                 const Assignment& ss0,
                                 const Ints &i0,
                                 const Assignment& ss1,
                                 const Ints &i1) ;

IMPDOMINOEXPORT void
load_union(const Subset &s0, const Subset &s1,
           AssignmentContainer* nd0, AssignmentContainer* nd1,
           const EdgeData &ed,
           size_t max,
           AssignmentContainer* out);

inline void update_list_subset_filter_table(ListSubsetFilterTable *lsft,
                                     const Subset &s,
                                     AssignmentContainer* ac) {
  for (unsigned int i=0; i< s.size(); ++i) {
    boost::dynamic_bitset<> bs(lsft->get_number_of_particle_states(s[i]));
    bs.reset();
    Ints cur= ac->get_particle_assignments(i);
    for (unsigned int j=0; j< cur.size(); ++j) {
      bs.set(cur[j]);
    }
    lsft->mask_allowed_states(s[i], bs);
  }
}


IMPDOMINO_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO_INTERNAL_INFERENCE_UTILITY_H */
