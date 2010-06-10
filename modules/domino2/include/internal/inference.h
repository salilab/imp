/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_INTERNAL_INFERENCE_H
#define IMPDOMINO2_INTERNAL_INFERENCE_H

#include "../domino2_config.h"
#include "../utility.h"

#include <IMP/Model.h>
#include <IMP/ScoreState.h>
#include <IMP/Restraint.h>

#include <vector>
#include <debug/vector>
#include <map>
#include <sstream>
IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

class IncompleteStates {
std::vector<int> v_;
  int compare(const IncompleteStates &o) const {
    IMP_USAGE_CHECK(o.size() == size(), "Sizes don't match");
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o[i]) return -1;
      else if (v_[i] > o[i]) return 1;
    }
    return 0;
  }
  typedef IncompleteStates This;
public:
  IncompleteStates(unsigned int sz): v_(sz, -1){}
  IMP_COMPARISONS;
  int operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < v_.size(), "Out of range");
    return v_[i];
  }
  int& operator[](unsigned int i) {
    IMP_USAGE_CHECK(i < v_.size(), "Out of range");
    return v_[i];
  }
  unsigned int size() const {
    return v_.size();
  }
  IMP_SHOWABLE_INLINE({
      out << "[";
      for (unsigned int i=0; i< size(); ++i) {
        if (v_[i] >= 0) out << v_[i];
        else out << "-";
        if (i != size()-1) out << " ";
      }
      out << "]";
    });
  operator SubsetState() const {
    SubsetState ret(size());
    for (unsigned int i=0; i< size(); ++i) {
      IMP_USAGE_CHECK(v_[i] >=0, "Not a full state");
      ret[i]=v_[i];
    }
    return ret;
  }
};
IMP_OUTPUT_OPERATOR(IncompleteStates);

inline bool get_is_match(const IncompleteStates &a,
                         const IncompleteStates &b) {
  IMP_INTERNAL_CHECK(a.size() == b.size(), "Sizes don't match");
  for (unsigned int i=0; i< a.size(); ++i) {
    if (a[i]<0 || b[i] < 0) continue;
    if (a[i] != b[i]) return false;
  }
  return true;
}
inline IncompleteStates get_merged(const IncompleteStates &a,
                                   const IncompleteStates &b) {
  IMP_INTERNAL_CHECK(get_is_match(a,b), "States don't match");
  IncompleteStates ret(a.size());
  for (unsigned int i=0; i< a.size(); ++i) {
    ret[i]=std::max(a[i], b[i]);
  }
  return ret;
}

typedef std::map<Particle*, int> ParticleIndex;
ParticleIndex get_index(Subset *s);



struct NodeData {
  // already have subset
  typedef std::map<SubsetState, double> Scores;
  Scores scores;
};

inline std::ostream &operator<<(std::ostream &out, const NodeData &nd) {
  for (NodeData::Scores::const_iterator it = nd.scores.begin();
       it != nd.scores.end(); ++it) {
    out << "(" << it->first << " = " << it->second << ")"
        << std::endl;;
  }
  return out;
}

struct EdgeData: public NodeData {
  Pointer<Subset> subset;
  Ints global_index;
};

struct PropagatedData {
  // already have subset
  typedef std::map<IncompleteStates, double> Scores;
  Scores scores;
};

inline std::ostream &operator<<(std::ostream &out, const PropagatedData &nd) {
  for (PropagatedData::Scores::const_iterator it = nd.scores.begin();
       it != nd.scores.end(); ++it) {
    out << "(" << it->first << " = " << it->second << ")"
        << std::endl;;
  }
  return out;
}


inline IncompleteStates get_incomplete_states(const ParticleIndex &pi,
                                              Subset* subset,
                                              const SubsetState &cs) {
  IncompleteStates ret(pi.size());
  for (unsigned int i=0; i< subset->get_number_of_particles(); ++i) {
    int j= pi.find(subset->get_particle(i))->second;
    ret[j]= cs[i];
  }
  return ret;
}

IMPDOMINO2EXPORT PropagatedData
get_propagated_data(const ParticleIndex &all_particles,
                    Subset* subset,
                    const NodeData &cs);

IMPDOMINO2EXPORT NodeData get_node_data(Subset *s,
                       const SubsetEvaluatorTable *eval,
                       const SubsetStatesTable *states);





IMPDOMINO2EXPORT PropagatedData get_best_conformations(const SubsetGraph &jt,
                                      int root,
                                      Subset* all_particles,
                                      const SubsetEvaluatorTable *eval,
                                      const SubsetStatesTable *states,
                                      double max_score);


IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO2_INTERNAL_INFERENCE_H */
