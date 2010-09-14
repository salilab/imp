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
      /*std::cout << "Found " << data_.get_number_of_restraints()\
        << " restraints."
        << std::endl;*/
    }
    IMP_SUBSET_FILTER(RestraintScoreSubsetFilter);
  };

  bool RestraintScoreSubsetFilter::get_is_ok(const SubsetState &state) const{
    IMP_OBJECT_LOG;
    const bool ok=data_.get_is_ok(state);
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
  SubsetFilter *ret
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
  std::vector<std::pair<unsigned int, Ints> > filters
    = get_filters(s, excluded, pst_, pairs_);
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


IMPDOMINO2_END_NAMESPACE
