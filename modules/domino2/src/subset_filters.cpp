/**
 *  \file domino2/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/domino2_config.h>
#include <IMP/domino2/DominoSampler.h>
#include <map>
#include <set>
#include <boost/version.hpp>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/particle_states.h>
#include <IMP/core/XYZ.h>



IMPDOMINO2_BEGIN_NAMESPACE
SubsetFilter::SubsetFilter(std::string name): Object(name){}
SubsetFilter::~SubsetFilter(){}
SubsetFilterTable::~SubsetFilterTable(){}



/***************************SCORE ******************************/

namespace {

  class  RestraintScoreSubsetFilter: public SubsetFilter {
    Pointer<const ModelSubsetEvaluatorTable> keepalive_;
    const internal::SubsetData &data_;
    double max_;
#if IMP_BUILD < IMP_FAST
    mutable IMP::internal::Set<SubsetState> called_;
#endif
  public:
    RestraintScoreSubsetFilter(const ModelSubsetEvaluatorTable *t,
                               const internal::SubsetData &data,
                               double max):
      SubsetFilter("Restraint score filter"),
      keepalive_(t), data_(data),
      max_(max) {
      /*std::cout << "Found " << data_.get_number_of_restraints()\
        << " restraints."
        << std::endl;*/
    }
    IMP_SUBSET_FILTER(RestraintScoreSubsetFilter);
  };

  bool RestraintScoreSubsetFilter::get_is_ok(const SubsetState &state) const{
    IMP_IF_CHECK(USAGE) {
      IMP_USAGE_CHECK(called_.find(state)
                      == called_.end(),
                      "Already called with " << state << " for "
                      << data_.get_subset());
#if IMP_BUILD < IMP_FAST
      called_.insert(state);
#endif
    }
    IMP_OBJECT_LOG;
    set_was_used(true);
    const bool ok=data_.get_is_ok(state, max_);
    IMP_LOG(VERBOSE, "For subset " << data_.get_subset()
            << (ok?" accepted":" rejected")
            << " state " << state << std::endl);
    return ok;
  }

  void RestraintScoreSubsetFilter::do_show(std::ostream &out) const{}
}

double RestraintScoreSubsetFilter::get_strength() const {
  return 1-std::pow(.5, static_cast<int>(data_.get_number_of_restraints()));
}

RestraintScoreSubsetFilterTable::StatsPrinter::~StatsPrinter() {
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Resraint filtration statistics (attempts, passes):\n");
    for (unsigned int i=0; i< get()->data_.rdata_.size(); ++i) {
      std::pair<int,int> stat= get()->data_.rdata_[i].get_statistics();
      if (stat.first >0) {
        IMP_LOG(TERSE, "  \""
                << get()->data_.rdata_[i].get_restraint()->get_name()
                << "\" " << stat.first << " " << stat.second << std::endl);
      }
    }
  }
}


RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(ModelSubsetEvaluatorTable *eval):
  mset_(eval)
{
}

SubsetFilter*
RestraintScoreSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  set_was_used(true);
  // if there are no restraints just here, the total score can't change
  if (mset_->data_.get_subset_data(s, excluded)
      .get_number_of_restraints() ==0) return NULL;
  SubsetFilter* ret
    = new RestraintScoreSubsetFilter(mset_,
                                     mset_->data_.get_subset_data(s,
                                                                  excluded),
                                     mset_->data_.get_model()
                                     ->get_maximum_score());
  ret->set_log_level(get_log_level());
  return ret;
}

void RestraintScoreSubsetFilterTable::do_show(std::ostream &out) const {
}


/************************************ PERMUTATION *****************************/

namespace {
  template <bool EQ>
  class  PermutationSubsetFilter: public SubsetFilter {
    std::vector<std::pair<unsigned int, Ints> > exclusions_;
  public:
    PermutationSubsetFilter(const std::vector<std::pair<unsigned int,
                            Ints> > &excl): exclusions_(excl) {
    }
    IMP_SUBSET_FILTER(PermutationSubsetFilter);
  };
  template <bool EQ>
  double PermutationSubsetFilter<EQ>::get_strength() const {
    double r=1;
    for (unsigned int i=0; i< exclusions_.size(); ++i) {
      for (unsigned int j=0; j < exclusions_[i].second.size(); ++j) {
        r*=(EQ?.1:.9);
      }
    }
    return 1-r;
  }
  template <bool EQ>
  bool PermutationSubsetFilter<EQ>::get_is_ok(const SubsetState &state) const{
    IMP_OBJECT_LOG;
    set_was_used(true);
    for (unsigned int i=0; i< exclusions_.size(); ++i) {
      for (unsigned int j=0; j< exclusions_[i].second.size(); ++j) {
        int a=state[exclusions_[i].first], b= state[exclusions_[i].second[j]];
        if ((EQ && a != b) || (!EQ && a==b)) {
          return false;
        }
      }
    }
    return true;
  }
  template <bool EQ>
  void PermutationSubsetFilter<EQ>::do_show(std::ostream &out) const{}


  bool is_excluded(int ia, int ib,
                   const std::vector<std::vector<bool> > &ebv) {
    for (unsigned int i=0; i< ebv.size(); ++i) {
      if (ebv[i][ia] && ebv[i][ib]) return true;
    }
    return false;
  }

  bool get_is_same(int ia, int ib,
               Particle *a, Particle *b,
               const ParticleStatesList &pss,
               const ParticlePairsTemp &pairs) {
    IMP_USAGE_CHECK( a< b, "Out of order particles");
    if (!pss.empty()) {
      return pss[ia] == pss[ib];
    } else {
      // could accelerate this if needed
      for (unsigned int i=0; i< pairs.size(); ++i) {
        if (pairs[i][0] == a && pairs[i][1]==b) {
          return true;
        }
      }
      return false;
    }
  }

  std::vector<std::pair<unsigned int, Ints> >
  get_filters(const Subset &s,
              const Subsets &excluded,
              ParticleStatesTable *pst,
              const ParticlePairsTemp &pairs) {
    std::vector<std::pair<unsigned int, Ints> > filters;
    ParticleStatesList pss;
    if (pst) {
      pss.resize(s.size());
      for (unsigned int i=0; i< s.size(); ++i) {
        pss[i]= pst->get_particle_states(s[i]);
      }
    }
    std::vector<std::vector<bool> > ebv(excluded.size(),
                                        std::vector<bool>(s.size(), false));
    {
      for (unsigned int i=0; i< s.size(); ++i) {
        for (unsigned int j=0; j< excluded.size(); ++j) {
          for (unsigned int k=0; k< excluded[j].size(); ++k) {
            if (s[i] == excluded[j][k]) {
              ebv[j][i]=true;
              break;
            }
          }
        }
      }
    }
    for (unsigned int i=0; i< s.size(); ++i) {
      for (unsigned int j=0; j<i; ++j) {
        if (get_is_same(j, i, s[j], s[i], pss, pairs)
            && !is_excluded(i,j, ebv)) {
          if (!filters.empty() && filters.back().first == i) {
            filters.back().second.push_back(j);
          } else {
            filters.push_back(std::make_pair(i, Ints(1, j)));
          }
        }
      }
    }
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      if (pst) {
        for (unsigned int i=0; i< filters.size(); ++i) {
          for (unsigned int j=0; j< filters[i].second.size(); ++j) {
            IMP_INTERNAL_CHECK(pst->get_particle_states(s[filters[i].first])
                        == pst->get_particle_states(s[filters[i].second[j]]),
                               "They don't match :-( ");
          }
        }
      }
    }
    return filters;
  }

  ParticlePairsTemp fixup(ParticlePairsTemp ppt) {
    for (unsigned int i=0; i< ppt.size(); ++i) {
      if (ppt[i][0] > ppt[i][1]) {
        ppt[i]= ParticlePair(ppt[i][1], ppt[i][0]);
      }
    }
    return ppt;
  }

}


PermutationSubsetFilterTable
::PermutationSubsetFilterTable(ParticleStatesTable *pst): pst_(pst)
{
}

PermutationSubsetFilterTable
::PermutationSubsetFilterTable(const ParticlePairsTemp &pairs):
  pairs_(fixup(pairs))
{
}

SubsetFilter*
PermutationSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  set_was_used(true);
  std::vector<std::pair<unsigned int, Ints> > filters
    = get_filters(s, excluded, pst_, pairs_);
  if (filters.empty()) return NULL;
  return new PermutationSubsetFilter<false>(filters);
}

void PermutationSubsetFilterTable::do_show(std::ostream &out) const {
}




EqualitySubsetFilterTable
::EqualitySubsetFilterTable(ParticleStatesTable *pst): pst_(pst)
{
}

EqualitySubsetFilterTable
::EqualitySubsetFilterTable(const ParticlePairsTemp &pairs):
  pairs_(fixup(pairs))
{
}

SubsetFilter*
EqualitySubsetFilterTable::get_subset_filter(const Subset &s,
                                             const Subsets &excluded) const {
  std::vector<std::pair<unsigned int, Ints> > filters
    = get_filters(s, excluded, pst_, pairs_);
  return new PermutationSubsetFilter<true>(filters);
}

void EqualitySubsetFilterTable::do_show(std::ostream &out) const {
}




// **************************************** List ********************


namespace {

  class  ListSubsetFilter: public SubsetFilter {
    Pointer<const ListSubsetFilterTable> keepalive_;
    Ints pos_;
    Ints indexes_;
  public:
    ListSubsetFilter(const ListSubsetFilterTable *ka,
                     const Ints pos, const Ints indexes):
      SubsetFilter("List score filter"),
      keepalive_(ka), pos_(pos), indexes_(indexes) {
    }
    IMP_SUBSET_FILTER(ListSubsetFilter);
  };

  bool ListSubsetFilter::get_is_ok(const SubsetState &state) const{
    ++keepalive_->num_test_;
    for (unsigned int i=0; i < pos_.size(); ++i) {
      if (!keepalive_->states_[indexes_[i]].test(state[pos_[i]])) {
        IMP_LOG(VERBOSE, "Rejecting state " << state
                << " due to particle " << pos_[i] << std::endl);
        return false;
      }
    }
    ++keepalive_->num_ok_;
    return true;
  }

  void ListSubsetFilter::do_show(std::ostream &out) const{}
}

double ListSubsetFilter::get_strength() const {
  return 1-std::pow(.5, static_cast<int>(pos_.size()));
}



ListSubsetFilterTable
::ListSubsetFilterTable(ParticleStatesTable *pst): pst_(pst)
{
  num_ok_=0;
  num_test_=0;
}

int ListSubsetFilterTable
::get_index(Particle*p) const {
  if (map_.find(p) == map_.end()) {
    map_[p]= states_.size();
    Pointer<ParticleStates> ps= pst_->get_particle_states(p);
    const int num=ps->get_number_of_particle_states();
    states_.push_back(boost::dynamic_bitset<>(num);
    states_.back().set();
    return states_.size()-1;
  } else {
    return map_.find(p)->second;
  }
}

SubsetFilter*
ListSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  ParticlesTemp cur(s.begin(), s.end());
  Ints pos(cur.size());
  Ints indexes(cur.size());
  for (unsigned int i=0; i< cur.size(); ++i) {
    for (unsigned int j=0; j< s.size(); ++j) {
      if (s[j]== cur[i]) {
        pos[i]=j;
        break;
      }
    }
    indexes[i]= get_index(cur[i]);
  }
  return new ListSubsetFilter(this, pos, indexes);
}

void ListSubsetFilterTable::intersect(Particle*p,
                                      const boost::dynamic_bitset<> &s) {
  int index= get_index(p);
  states_[index] &= s;
}

void ListSubsetFilterTable::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
