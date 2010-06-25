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
#include "../utility.h"
#include "../particle_states.h"
//#include "inference.h"
#include <IMP/Restraint.h>


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE
/*
  data for a subset:
  - list f restraints
  - mapping of restraint subset for each subset
  data for each restraint:
  - cache of scores for subset state
 */
class RestraintData {
  typedef  std::map<SubsetState, double> Scores;
  mutable Scores scores_;
  Pointer<Restraint> r_;
  double weight_;
  double max_;
public:
  RestraintData(Restraint *r,
                double weight): r_(r), weight_(weight),
                                max_(std::numeric_limits<double>::max()){}
  void set_max(double max) { max_=max;}
  Restraint *get_restraint() const {return r_;}
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
      for (unsigned int i=0; i< ps.size(); ++i) {
        pst->get_particle_states(ps[i])->load_state(state[i], ps[i]);
      }
      double score= r_->evaluate(false)*weight_;
      if (score >max_) {
        score=std::numeric_limits<double>::max();
      }
      scores_[state]=score;
      /*std::cout << "Computed score for " << r_->get_name()
        << " on " << state << "= " << score << std::endl;*/
      return score;
    }
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
};

class IMPDOMINO2EXPORT ModelData {
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
  friend class SubsetData;
  mutable Pointer<Model> m_;
  std::vector<RestraintData> rdata_;
  std::vector<ParticlesTemp> dependencies_;
  Pointer<ParticleStatesTable> pst_;
  mutable std::map<const SubsetID, SubsetData> sdata_;
public:
  ModelData(Model *m, const DependencyGraph &dg,
            ParticleStatesTable* pst);
  void set_sampler(const Sampler *s);
  const SubsetData &get_subset_data(const Subset &s,
                                    const Subsets &exclude=Subsets()) const;
};

IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO2_INTERNAL_RESTRAINT_EVALUATOR_H */
