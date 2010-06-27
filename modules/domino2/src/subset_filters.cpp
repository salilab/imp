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
#include <IMP/domino2/internal/inference.h>
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
  public:
    RestraintScoreSubsetFilter(const ModelSubsetEvaluatorTable *t,
                               const internal::SubsetData &data,
                               double max): keepalive_(t), data_(data),
                                            max_(max) {
    }
    IMP_SUBSET_FILTER(RestraintScoreSubsetFilter);
  };

  bool RestraintScoreSubsetFilter::get_is_ok(const SubsetState &state) const{
    return data_.get_is_ok(state);
  }

  void RestraintScoreSubsetFilter::do_show(std::ostream &out) const{}
}


RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(ModelSubsetEvaluatorTable *eval,
                                  const Sampler *s):
  mset_(eval),
  max_(s->get_maximum_score())
{
  mset_->data_.set_sampler(s);
}

SubsetFilter*
RestraintScoreSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  return new RestraintScoreSubsetFilter(mset_,
                                        mset_->data_.get_subset_data(s,
                                                                     excluded),
                                        max_);
}

void RestraintScoreSubsetFilterTable::do_show(std::ostream &out) const {
}


/************************************ PERMUTATION *****************************/

namespace {

  class  PermutationSubsetFilter: public SubsetFilter {
    std::vector<std::pair<unsigned int, Ints> > exclusions_;
  public:
    PermutationSubsetFilter(const std::vector<std::pair<unsigned int,
                            Ints> > &excl): exclusions_(excl) {
    }
    IMP_SUBSET_FILTER(PermutationSubsetFilter);
  };

  bool PermutationSubsetFilter::get_is_ok(const SubsetState &state) const{
    for (unsigned int i=0; i< exclusions_.size(); ++i) {
      for (unsigned int j=0; j< exclusions_[i].second.size(); ++j) {
        if (state[exclusions_[i].first]== state[exclusions_[i].second[j]]) {
          return false;
        }
      }
    }
    return true;
  }

  void PermutationSubsetFilter::do_show(std::ostream &out) const{}
}


PermutationSubsetFilterTable
::PermutationSubsetFilterTable(ParticleStatesTable *pst): pst_(pst)
{
}

SubsetFilter*
PermutationSubsetFilterTable::get_subset_filter(const Subset &s,
                                                const Subsets &excluded) const {
  std::vector<ParticleStates*> ps;
  std::vector<bool> isexcluded;
  std::vector<std::pair<unsigned int, Ints> > filters;
  for (unsigned int i=0; i< s.size(); ++i) {
    ps.push_back(pst_->get_particle_states(s[i]));
    bool found=false;
    for (unsigned j=0; j< excluded.size() && !found; ++j) {
      if (std::binary_search(excluded[j].begin(),
                        excluded[j].end(), s[i])) {
        found=true;
        break;
      }
    }
    /* if (found) {
      IMP_LOG(VERBOSE, s[i]->get_name() << " is excluded " << std::endl);
      }*/
    isexcluded.push_back(found);
    for (unsigned int j=0; j< ps.size()-1; ++j) {
      if (ps.back() == ps[j] && (!isexcluded.back() || !isexcluded[j])) {
        /*IMP_LOG(VERBOSE, "found " << s[i]->get_name()
          << " " << s[j]->get_name() << std::endl);*/
        if (!filters.empty() && filters.back().first == i) {
          filters.back().second.push_back(j);
        } else {
          filters.push_back(std::make_pair(i, Ints(1, j)));
        }
      }
    }
  }
  /*IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Subset " << s << " with exclusions ");
    for (unsigned int i=0; i< excluded.size(); ++i) {
      IMP_LOG(VERBOSE, excluded[i] << " ");
    }
    IMP_LOG(VERBOSE, std::endl << " Got ");
    for (unsigned int i=0; i< filters.size(); ++i) {
      IMP_LOG(VERBOSE, filters[i].first << ": ");
      for (unsigned int j=0; j< filters[i].second.size(); ++j) {
        IMP_LOG(VERBOSE, filters[i].second[j] << " ");
      }
      IMP_LOG(VERBOSE, " & ");
    }
    IMP_LOG(VERBOSE, std::endl);
    }*/
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< filters.size(); ++i) {
      for (unsigned int j=0; j< filters[i].second.size(); ++j) {
      IMP_INTERNAL_CHECK(pst_->get_particle_states(s[filters[i].first])
                         == pst_->get_particle_states(s[filters[i].second[j]]),
                         "They don't match :-( ");
      }
    }
  }
  return new PermutationSubsetFilter(filters);
}

void PermutationSubsetFilterTable::do_show(std::ostream &out) const {
}
IMPDOMINO2_END_NAMESPACE
