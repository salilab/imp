/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_INTERNAL_RESTRAINT_EVALUATOR_H
#define IMPDOMINO2_INTERNAL_RESTRAINT_EVALUATOR_H

#include "../Subset.h"
#include "../SubsetState.h"
#include "../utility.h"
#include "../particle_states.h"
//#include "inference.h"
#include <IMP/Restraint.h>
#if IMP_BOOST_VERSION > 103900
#include <boost/unordered_map.hpp>
#else
#include <map>
#endif


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

template <class It>
void load_particle_states(It b, It e, const SubsetState &ss,
                          const ParticleStatesTable *pst) {
  IMP_USAGE_CHECK(std::distance(b,e) == ss.size(),
                  "Sizes don't match in load particle states");
  unsigned int i=0;
  for (It c=b; c != e; ++c) {
    pst->get_particle_states(*c)->load_state(ss[i], *c);
    ++i;
  }
}

/*
  data for a subset:
  - list f restraints
  - mapping of restraint subset for each subset
  data for each restraint:
  - cache of scores for subset state
 */
class RestraintData {
  //#if IMP_BOOST_VERSION > 103900
//typedef  boost::unordered_map<SubsetState, double> Scores;
//#else
  typedef std::map<SubsetState, double> Scores;
  //#endif
  mutable Scores scores_;
  Pointer<Restraint> r_;
  double weight_;
  double max_;
  mutable int filter_attempts_;
  mutable int filter_passes_;
public:
  RestraintData(Restraint *r,
                double weight): r_(r), weight_(weight),
                                max_(std::numeric_limits<double>::max()){
    filter_attempts_=0;
    filter_passes_=0;
  }
  void set_max(double max) { max_=max;}
  Restraint *get_restraint() const {return r_;}
  template <bool Filter>
  double get_score(ParticleStatesTable *pst,
                   const ParticlesTemp &ps,
                   const SubsetState &state) const {
    Scores::const_iterator it= scores_.find(state);
    if (it != scores_.end()) {
      /*std::cout << "Found cached score for " << r_->get_name()
        << " on " << state << "= " << it->second
        << "(" << it->first << ")" << std::endl;*/
      return it->second;
    } else {
      load_particle_states(ps.begin(), ps.end(), state, pst);
      double score= r_->evaluate(false)*weight_;
      if (Filter) {
        ++filter_attempts_;
        if (score >max_) {
          score=std::numeric_limits<double>::max();
        } else {
          ++filter_passes_;
        }
      }
      scores_[state]=score;
      /*std::cout << "Computed score for " << r_->get_name()
        << " on " << state << "= " << score << std::endl;*/
      return score;
    }
  }
  std::pair<int,int> get_statistics() const {
    return std::make_pair(filter_attempts_, filter_passes_);
  }
};

class ModelData;

class IMPDOMINO2EXPORT SubsetData {
  const ModelData *md_;
  Ints ris_;
  std::vector<Ints> indices_;
  Subset s_;
public:
  SubsetData(){}
  SubsetData(const ModelData *md,
             const Ints &ris,
             std::vector<Ints> indices,
             const Subset &s): md_(md), ris_(ris), indices_(indices), s_(s){}
  double get_score(const SubsetState &state) const;
  bool get_is_ok(const SubsetState &state) const;
  unsigned int get_number_of_restraints() const {
    return ris_.size();
  }
};

struct IMPDOMINO2EXPORT ModelData {
  struct SubsetID {
    const Subset s_;
    const Subsets excluded_;
    SubsetID(const Subset &s,
             const Subsets &excluded): s_(s), excluded_(excluded){}
    bool operator<(const SubsetID &o) const {
      if (s_ < o.s_) return true;
      else if (s_ > o.s_) return false;
      else if (excluded_.size() < o.excluded_.size()) return true;
      else if (excluded_.size() > o.excluded_.size()) return false;
      else {
        for (unsigned int i=0; i< excluded_.size(); ++i) {
          if (excluded_[i] < o.excluded_[i]) return true;
          else if (excluded_[i] > o.excluded_[i]) return false;
        }
        return false;
      }
    }
  };
  mutable Pointer<Model> m_;
  std::vector<RestraintData> rdata_;
  std::vector<ParticlesTemp> dependencies_;
  Pointer<ParticleStatesTable> pst_;
  mutable std::map<const SubsetID, SubsetData> sdata_;

  ModelData(Model *m, RestraintSet *rs,
            const DependencyGraph &dg,
            ParticleStatesTable* pst);
  void set_sampler(const Sampler *s);
  const SubsetData &get_subset_data(const Subset &s,
                                    const Subsets &exclude=Subsets()) const;
};

IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO2_INTERNAL_RESTRAINT_EVALUATOR_H */
