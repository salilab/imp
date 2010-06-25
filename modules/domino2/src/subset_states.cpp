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
SubsetStates::~SubsetStates(){}
SubsetStatesTable::~SubsetStatesTable(){}



namespace {
  typedef std::map<Particle*, Particle*> IParent;
  typedef std::map<Particle*, int> IRank;
  typedef boost::associative_property_map<IParent> Parent;
  typedef boost::associative_property_map<IRank > Rank;
  typedef boost::disjoint_sets<Rank, Parent> UF;


  template <class It>
  void permutation_initialize(It b, It e,
                              typename std::iterator_traits<It>::value_type beg,
                              typename std::iterator_traits<It>::value_type end)
  {
    for (It c= b; c!= e; ++c) {
      *c=beg+std::distance(c,e)-1;
    }
  }

  template <class It>
  void permutation_size(It b, It e,
                        typename std::iterator_traits<It>::value_type beg,
                        typename std::iterator_traits<It>::value_type end) {
    const unsigned int k= std::distance(b,e);
    unsigned int ret=end;
    for (unsigned int i=1; i< k; ++i) {
      ret*= end-i;
    }
    return ret;
  }

  template <class It>
  bool permutation_get_is_permutation(It b, It e,
                       typename std::iterator_traits<It>::value_type beg,
                       typename std::iterator_traits<It>::value_type end) {
    for (It c0= b; c0 != e; ++c0) {
      for (It c1= b; c1 != c0; ++c1) {
        if (*c0 == *c1) return false;
      }
    }
    return true;
  }

  template <class It>
  bool permutation_increment(It b, It e,
                             typename std::iterator_traits<It>::value_type beg,
                             typename std::iterator_traits<It>::value_type end)
  {
    It c=b;
    for (; c != e; ++c) {
      ++*c;
      if (*c==end) {
        *c=beg;
      } else {
        break;
      }
    }
    if (c==e) {
      *b=-1;
      return true;
    }
    return false;
  }

  template <class It>
  bool permutation_advance(It b, It e,
                           typename std::iterator_traits<It>::value_type beg,
                           typename std::iterator_traits<It>::value_type end) {
    do {
      if (permutation_increment(b,e,beg,end)) {
        permutation_initialize(b,e,beg,end);
        return true;
      }
    } while (!permutation_get_is_permutation(b,e,beg, end));
    return false;
  }

  template <class It>
  bool permutation_advance_i(It b, It e,
                             typename std::iterator_traits<It>::value_type beg,
                             typename std::iterator_traits<It>::value_type end,
                             unsigned int i) {
    for (It c= b; c < b+i; ++c) {
      *c= end-1;
    }
    return permutation_advance(b,e,beg,end);
  }


  template <class It>
  bool permutation_get_is_end(It b, It e,
                              typename std::iterator_traits<It>::value_type beg,
                              typename std::iterator_traits<It>::value_type end)
  {
    return *b==-1;
  }


  class  BranchAndBoundSubsetStates: public SubsetStates {
  public:
    void setup_filters(const Subset &s,
                       const SubsetFilterTables &sfts,
                       std::vector<SubsetFilters> &ses);
    std::vector<SubsetState> states_;
    BranchAndBoundSubsetStates(const Subset &s,
                               ParticleStatesTable *table,
                               const SubsetFilterTables &sft);
    IMP_SUBSET_STATES(BranchAndBoundSubsetStates);
  };


  void BranchAndBoundSubsetStates::setup_filters(const Subset &s,
                                                 const SubsetFilterTables &sfts,
                                              std::vector<SubsetFilters> &ses) {
    IMP_INTERNAL_CHECK(s.size()== ses.size(), "Sizes don't match");
    for (unsigned int j=0; j < ses.size(); ++j) {
      ParticlesTemp pt;
      if (j < s.size()) {
        pt= ParticlesTemp(s.begin()+j, s.end());
      }
      Subset s(pt, true);
      ParticlesTemp ept;
      if (j+1 < s.size()) {
        ept= ParticlesTemp(s.begin()+ j+1, s.end());
      }
      Subset es(ept, true);
      for (unsigned int i=0; i< sfts.size(); ++i) {
        //std::cout << "Getting evaluator for " << i << std::endl;
        SubsetFilter* se= sfts[i]->get_subset_filter(s,
                                                     Subsets(1,es));
        se->set_was_used(true);
        if (se) {
          ses[j].push_back(se);
        }
      }
    }
  }


  BranchAndBoundSubsetStates
  ::BranchAndBoundSubsetStates(const Subset &s,
                               ParticleStatesTable *table,
                               const SubsetFilterTables &sft):
    SubsetStates("BranchAndBoundSubsetStates on "+s.get_name()) {
    IMP_OBJECT_LOG;
    std::vector<SubsetFilters> filters(s.size());
    setup_filters(s, sft, filters);

    IMP_CHECK_OBJECT(table);
    // create lists

    unsigned int sz=s.size();
    Ints maxs(sz);
    for (unsigned int i=0; i< sz; ++i) {
      maxs[i]=table->get_particle_states(s[i])
        ->get_number_of_states();
    }
    Ints cur(sz, -1);
    unsigned int changed_digit=cur.size()-1;
    unsigned int current_digit=0;
    for (unsigned int i=0; i< cur.size(); ++i) {
      cur[i]=0;
    }
    goto filter;

  filter:
    //std::cout << "Filtering " << cur << " on " << changed_digit << std::endl;
    for (int i=changed_digit; i >=0; --i) {
      for (unsigned int j=0; j < filters[i].size(); ++j) {
        if (!filters[i][j]->get_is_ok(SubsetState(SubsetState(cur.begin()+i,
                                                              cur.end())))) {
          goto bad;
        }
      }
      continue;
    bad:
      //std::cout << "Failed at " << i << std::endl;
      current_digit=i;
      goto increment;
    }
    current_digit=0;
    IMP_LOG(VERBOSE, "Found " << cur << std::endl);
    states_.push_back(SubsetState(cur));
    goto increment;
  increment:
    //std::cout << "Incrementing " << cur << " on "
    //<< current_digit << std::endl;
    for (unsigned int i=0; i< current_digit; ++i) {
      cur[i]=0;
    }
    for (unsigned int i=current_digit; i < cur.size(); ++i) {
      ++cur[i];
      if (cur[i]==maxs[i]) {
        cur[i]=0;
      } else {
        changed_digit=i;
        goto filter;
      }
    }
    //done:
    std::sort(states_.begin(), states_.end());
  }



  unsigned int BranchAndBoundSubsetStates::get_number_of_states() const {
    return states_.size();
  }
  SubsetState BranchAndBoundSubsetStates::get_state(unsigned int i) const {
    return states_[i];
  }

  bool
  BranchAndBoundSubsetStates::get_is_state(const SubsetState &state) const {
    return std::binary_search(states_.begin(), states_.end(), state);
  }

  void BranchAndBoundSubsetStates::do_show(std::ostream &out) const{}

}


BranchAndBoundSubsetStatesTable
::BranchAndBoundSubsetStatesTable(ParticleStatesTable *pst,
                                  const SubsetFilterTables &sft):
  pst_(pst), sft_(sft){}


SubsetStates* BranchAndBoundSubsetStatesTable
::get_subset_states(const Subset&s) const {
  return new BranchAndBoundSubsetStates(s,
                                        pst_, sft_);
}

void BranchAndBoundSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
