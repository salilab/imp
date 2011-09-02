/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_RESTRAINT_EVALUATOR_H
#define IMPDOMINO_INTERNAL_RESTRAINT_EVALUATOR_H

#include "../Subset.h"
#include "../Assignment.h"
#include "../utility.h"
#include "../particle_states.h"
//#include "inference.h"
#include <IMP/Restraint.h>



IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

template <class It>
inline void load_particle_states(It b, It e, const Assignment &ss,
                                 const ParticleStatesTable *pst) {
  IMP_USAGE_CHECK(std::distance(b,e)
                  == static_cast< typename std::iterator_traits<It>
                  ::difference_type>(ss.size()),
                  "Sizes don't match in load particle states");
  unsigned int i=0;
  for (It c=b; c != e; ++c) {
    pst->get_particle_states(*c)->load_particle_state(ss[i], *c);
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
  typedef IMP::compatibility::map<Assignment, double> Scores;
  mutable Scores scores_;
  Pointer<Restraint> r_;
  double weight_;
  double max_;
  unsigned int max_cache_;
  mutable int filter_attempts_;
  mutable int filter_passes_;
  template <bool Filter>
  double compute_score(ParticleStatesTable *pst,
                       const ParticlesTemp &ps,
                       const Assignment &state) const {
    load_particle_states(ps.begin(), ps.end(), state, pst);
    RestraintsTemp rs(1, r_);
    double score= r_->get_model()->evaluate_if_good(rs, false)[0];
    if (Filter) {
      if (score >max_) {
        score=std::numeric_limits<double>::max();
      } else {
      }
    }
    /*std::cout << "Computed score for " << r_->get_name()
      << " on " << state << "= " << score << std::endl;*/
    return score;
  }
  void clean_cache() const {
    if (scores_.size() > max_cache_) {
      IMP_LOG(VERBOSE, "Cleaning cache from " << scores_.size());
      int numr= std::min<int>(max_cache_*.3+1, scores_.size());
      for (int i=0; i< numr; ++i) {
        scores_.erase(scores_.begin());
      }
      IMP_LOG(VERBOSE, " to " << scores_.size() << std::endl);
    }
  }
public:
  RestraintData(Restraint *r): r_(r),
                               max_(std::numeric_limits<double>::max()){
    filter_attempts_=0;
    filter_passes_=0;
    max_cache_=std::numeric_limits<unsigned int>::max();
  }
  void set_score(const Assignment &ss, double s) {
    IMP_USAGE_CHECK(scores_.find(ss) == scores_.end(),
                    "Cannot preload scores twice for state "
                    << ss);
    scores_[ss]=s;
    clean_cache();
  }
  void set_max(double max) { max_=max;}
  Restraint *get_restraint() const {return r_;}
  template <bool Filter>
  double get_score(ParticleStatesTable *pst,
                   const ParticlesTemp &ps,
                   const Assignment &state) const {
    Scores::const_iterator it;
    if (max_cache_>0) {
      it= scores_.find(state);
    } else {
      it= scores_.end();
    }
    if (max_cache_>0 && it != scores_.end()) {
      /*std::cout << "Found cached score for " << r_->get_name()
        << " on " << state << "= " << it->second
        << "(" << it->first << ")" << std::endl;*/
      double score= it->second;
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        double cscore= compute_score<Filter>(pst, ps, state);
        IMP_INTERNAL_CHECK((score >= max_ && cscore >= max_)
                           || (score < max_ && cscore < max_)
                           || (score-cscore) < .01,
                           "Scores don't match their side of max: "
                           << score << " vs " << cscore);
        if (score < max_) {
          IMP_INTERNAL_CHECK(std::abs(score-cscore)
                             < .1*std::abs(score+cscore) +.1,
                             "Scores don't match: " << score
                             << " vs " << cscore<<" test: " <<
                             score-cscore<<" "<<
                             .1*(score+cscore) +.1<<std::endl);
        }
      }
      return score;
    } else {
      double score= compute_score<Filter>(pst, ps, state);
      IMP_LOG(VERBOSE, "State " << state << " of particles "
              << ps << " has score "
              << score << " for restraint " << r_->get_name()
              << std::endl);
      if (Filter) {
        ++filter_attempts_;
        if (score >max_) {
        } else {
          ++filter_passes_;
        }
      }
      if (max_cache_>0) {
        scores_[state]=score;
        clean_cache();
      }
      return score;
    }
  }
  std::pair<int,int> get_statistics() const {
    return std::make_pair(filter_attempts_, filter_passes_);
  }
  void set_max_cache(unsigned int ma) {
    max_cache_=ma;
  }
};

struct ModelData;

class IMPDOMINOEXPORT SubsetData {
  const ModelData *md_;
  Ints ris_;
  compatibility::checked_vector<Ints> indices_;
  compatibility::checked_vector<std::pair<double, Ints> > set_ris_;
  compatibility::checked_vector< compatibility::checked_vector<Ints> >
    set_indices_;
  compatibility::checked_vector<Floats> set_weights_;
  Subset s_;
 public:
  SubsetData(){}
  SubsetData(const ModelData *md,
             const Ints &ris,
             const compatibility::checked_vector<std::pair< double, Ints> >
             &set_ris,
             compatibility::checked_vector<Ints> indices,
     const compatibility::checked_vector<compatibility::checked_vector<Ints> >&
             set_indices,
             const compatibility::checked_vector<Floats>& set_weights,
             const Subset &s): md_(md), ris_(ris),
    indices_(indices),
    set_ris_(set_ris), set_indices_(set_indices), set_weights_(set_weights),
    s_(s){}
  double get_score(const Assignment &state) const ;
  bool get_is_ok(const Assignment &state, double max) const ;
  unsigned int get_number_of_restraints() const {
    return ris_.size();
  }
  unsigned int get_number_of_total_restraints() const {
    return set_ris_.size()+ ris_.size();
  }
  Subset get_subset() const {return s_;}
  RestraintsTemp get_restraints() const;
  Floats get_scores(const Assignment &state) const;
};

struct IMPDOMINOEXPORT ModelData: public RefCounted {
  struct PreloadData {
    Subset s;
    Assignments sss;
    Floats scores;
  };
  typedef IMP::compatibility::map<Restraint*, PreloadData> Preload;
  Preload preload_;
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

  bool initialized_;
  Restraints rs_;
  unsigned int max_cache_;
  compatibility::checked_vector<RestraintData> rdata_;
  compatibility::checked_vector<std::pair<double, Ints> > sets_;
  compatibility::checked_vector<Floats> set_weights_;
  compatibility::checked_vector<Subset> dependencies_;
  Pointer<ParticleStatesTable> pst_;
  mutable std::map<const SubsetID, SubsetData> sdata_;

  void validate() const;

  void initialize();

  ModelData(const RestraintsTemp &rs,
            ParticleStatesTable* pst);
  Model *get_model() const {
    return rs_[0]->get_model();
  }
  unsigned int get_number_of_restraints(const Subset &s,
                                        const Subsets &exclusions) const;
  const SubsetData &get_subset_data(const Subset &s,
                                    const Subsets &exclude=Subsets()) const;
  void add_score(Restraint *r, const Subset &subset,
                 const Assignment &state, double score);
  void set_use_caching(bool tf);
  void set_maximum_number_of_cache_entries(int max) {
    max_cache_=max;
  }
  IMP_REF_COUNTED_DESTRUCTOR(ModelData);
};



inline Floats SubsetData::get_scores(const Assignment &state) const {
  Floats scores;
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    Assignment ss(ssi);
    ParticlesTemp ps(ss.size());
    for (unsigned int j=0; j< ss.size(); ++j) {
      ps[j]= s_[indices_[i][j]];
    }
    double ms=md_->rdata_[ris_[i]].get_score<false>(md_->pst_,
                                                    ps, ss);
    scores.push_back(ms);
  }
  return scores;
}

inline double SubsetData::get_score(const Assignment &state) const {
  Floats r=get_scores(state);
  return std::accumulate(r.begin(), r.end(), 0.0);
}
inline bool SubsetData::get_is_ok(const Assignment &state, double max) const {
  double total=0;
  for (unsigned int i=0; i< ris_.size(); ++i) {
    Ints ssi(indices_[i].size());
    for (unsigned int j=0; j< ssi.size();++j) {
      ssi[j]= state[indices_[i][j]];
    }
    Assignment ss(ssi);
    ParticlesTemp ps(ss.size());
    for (unsigned int j=0; j< ss.size(); ++j) {
      ps[j]= s_[indices_[i][j]];
    }
    double ms=md_->rdata_[ris_[i]].get_score<true>(md_->pst_,
                                                   ps, ss);
    if (ms >= std::numeric_limits<double>::max()) {
      return false;
    }
    total+=ms;
    if (total > max) {
      return false;
    }
  }
  for (unsigned int h=0; h < set_ris_.size(); ++h) {
    double set_total=0;
    for (unsigned int i=0; i< set_ris_[h].second.size(); ++i) {
      Ints ssi(set_indices_[h][i].size());
      for (unsigned int j=0; j< ssi.size();++j) {
        ssi[j]= state[set_indices_[h][i][j]];
      }
      Assignment ss(ssi);
      ParticlesTemp ps(ss.size());
      for (unsigned int j=0; j< ss.size(); ++j) {
        ps[j]= s_[set_indices_[h][i][j]];
      }
      double ms=md_->rdata_[set_ris_[h].second[i]].get_score<true>(md_->pst_,
                                                                   ps, ss);
      set_total+=ms *set_weights_[h][i];
      if (set_total > set_ris_[h].first) {
        return false;
      }
    }
  }
  return true;
}
IMPDOMINO_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO_INTERNAL_RESTRAINT_EVALUATOR_H */
