/**
 *  \file domino2/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/domino2_config.h>
#include <IMP/domino2/DominoSampler.h>
#include <IMP/internal/map.h>
#include <set>
#include <boost/version.hpp>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/particle_states.h>
#include <IMP/domino2/internal/inference.h>
#include <IMP/core/XYZ.h>
#include <boost/iterator/permutation_iterator.hpp>


IMPDOMINO2_BEGIN_NAMESPACE
SubsetStates::~SubsetStates(){}
SubsetStatesTable::~SubsetStatesTable(){}

typedef std::vector<int> Ints;

namespace {
  typedef IMP::internal::Map<Particle*, Particle*> IParent;
  typedef IMP::internal::Map<Particle*, int> IRank;
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
  template <class It>
  Subset get_sub_subset(const Subset &s, It b, It e) {
    if (b==e) return Subset();
    ParticlesTemp pt(boost::make_permutation_iterator(s.begin(), b),
                     boost::make_permutation_iterator(s.begin(), e));
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< std::distance(b,e); ++i) {
        IMP_INTERNAL_CHECK(pt[i]==s[*(b+i)],
                           "Do not match at " << i
                           << " got " << pt[i] << " expected "
                           << s[*(b+i)] << " for " << s);
      }
    }
    Subset rs(pt);
    /*std::cout << "SubSubset " << s << " ";
    for (It c= b; c!= e; ++c) {
      std::cout << *c << " ";
    }
    std::cout << " is " << rs << std::endl;*/
    return rs;
  }
  template <class Ints>
 SubsetState get_sub_subset_state(const Subset &s, const Ints &ss,
                                  const Subset sub_s) {
   Ints ret(sub_s.size());
   for (unsigned int i=0; i< sub_s.size(); ++i) {
     for (unsigned int j=0; j< s.size(); ++j) {
       if (sub_s[i] == s[j]) {
         ret[i]= ss[j];
       }
     }
   }
   return SubsetState(ret);
  }




  // Branch and bound.....

  class  BranchAndBoundSubsetStates: public SubsetStates {
  public:
    void setup_filters(const Subset &s,
                       const Ints &order,
                       const SubsetFilterTables &sfts,
                       std::vector<SubsetFilters> &ses);
    std::vector<SubsetState> states_;
    BranchAndBoundSubsetStates(const Subset &s,
                               ParticleStatesTable *table,
                               const SubsetFilterTables &sft);
    IMP_SUBSET_STATES(BranchAndBoundSubsetStates);
  };


  void BranchAndBoundSubsetStates::setup_filters(const Subset &s,
                                                 const Ints &order,
                                                 const SubsetFilterTables &sfts,
                                              std::vector<SubsetFilters> &ses) {
    IMP_INTERNAL_CHECK(s.size()== ses.size(), "Sizes don't match");
    for (unsigned int j=0; j < ses.size(); ++j) {
      Subset cs= get_sub_subset(s, order.begin()+j, order.end());
      if (cs.size()==0) continue;
      Subset es= get_sub_subset(s, order.begin()+j+1, order.end());
      for (unsigned int i=0; i< sfts.size(); ++i) {
        /*std::cout << "Getting evaluator for " << j
          << " on " << cs << " and " << es << std::endl;*/
        SubsetFilter* se= sfts[i]->get_subset_filter(cs,
                                                     Subsets(1,es));
        if (se) {
          se->set_was_used(true);
          ses[j].push_back(se);
        }
      }
    }
  }

  namespace {
    void initialize_order(const Subset &s,
                          const SubsetFilterTables &sft,
                          Ints &order,
                          std::vector<SubsetFilters>& filters,
                          std::vector<Subset>& filter_subsets) {
      Ints remaining;
      for (unsigned int i=0; i< s.size(); ++i) {
        remaining.push_back(i);
      }
      for (unsigned int i=0; i< s.size(); ++i) {
        double max_restraint=-1;
        int max_j=-1;
        SubsetFilters max_filters;
        Subset max_subset;
        Subset all_remaining=get_sub_subset(s, remaining.begin(),
                                            remaining.end());
        Ints before;
        Ints after(remaining.rbegin(), remaining.rend());
        for (unsigned int j=0; j< remaining.size(); ++j) {
          //std::cout << "Trying " << remaining[j] << std::endl;
          int cur= after.back();
          IMP_INTERNAL_CHECK(remaining[j]== cur,
                             "Do not match " << remaining[j] << " and " << cur);
          after.pop_back();
          Ints excluded_order(before);
          excluded_order.insert(excluded_order.end(),
                                after.begin(), after.end());
          Subset excluded= get_sub_subset(s, excluded_order.begin(),
                                          excluded_order.end());
          // ask all tables about subset of taken+this particle - taken
          double cur_restraint=1;
          SubsetFilters cur_filters;
          for (unsigned int i=0; i < sft.size(); ++i) {
            /*std::cout << "Creating filter on "
              << all_remaining << " " << excluded
              << std::endl;*/
            SubsetFilter *cur_filterp
              =sft[i]->get_subset_filter(all_remaining,
                                         Subsets(1,
                                                 excluded));
            if (cur_filterp) {
              Pointer<SubsetFilter> cur_filter=cur_filterp;
              cur_filter->set_was_used(true);
              double str=cur_filter->get_strength();
              IMP_USAGE_CHECK(str >=0 && str <=1, "Strength is out of range "
                              << str);
              std::cout << "strength is " << str << std::endl;
              cur_restraint*= 1-str;
              cur_filters.push_back(cur_filter);
            }
          }
          std::cout << "Of " << s[remaining[j]]->get_name()
                    << " plus " << excluded << " got strength " << cur_restraint
                    << std::endl;
          if (cur_restraint >= max_restraint) {
            max_restraint=cur_restraint;
            max_j=j;
            max_filters=cur_filters;
            max_subset=all_remaining;
          }
          before.push_back(cur);
        }
        order.push_back(remaining[max_j]);
        filters.push_back(max_filters);
        filter_subsets.push_back(max_subset);
        remaining.erase(remaining.begin()+max_j);
        /*std::cout << "Remaining is ";
        for (unsigned int i=0; i< remaining.size(); ++i) {
          std::cout << remaining[i] << " ";
        }
        std::cout << std::endl;*/
      }
      IMP_LOG(TERSE, "Order for " << s << " is ");
      for (unsigned int i=0; i< order.size(); ++i) {
        IMP_LOG(TERSE,  order[i] << " ");
      }
      IMP_LOG(TERSE, std::endl);
    }
  }
  BranchAndBoundSubsetStates
  ::BranchAndBoundSubsetStates(const Subset &s,
                               ParticleStatesTable *table,
                               const SubsetFilterTables &sft):
    SubsetStates("BranchAndBoundSubsetStates on "+s.get_name()) {
    //std::cout << "Searching order for " << s << std::endl;
    Ints order;
    std::vector<SubsetFilters> filters;
    std::vector<Subset> filter_subsets;

    initialize_order(s, sft, order, filters, filter_subsets);

    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      std::set<int> taken(order.begin(), order.end());
      IMP_INTERNAL_CHECK(taken.size() == order.size(),
                         "Duplicate elements in order "
                         << taken.size() << " " << order.size());
    }
    IMP_OBJECT_LOG;
    IMP_LOG(TERSE, "Enumerating states for " << s << "..." << std::flush);

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

    /*IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      Ints state;
      std::map<Particle*, int> map;
      for (unsigned int i=0; i< s.size(); ++i) {
        state.push_back(i);
        map[s[i]]=i;
      }
      IMP_LOG(VERBOSE,"Order is: ");
      for (unsigned int i=0; i< order.size(); ++i) {
        IMP_LOG(VERBOSE, order[i] << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
      for (unsigned int i=0; i< s.size(); ++i) {
        {
          //Ints pstate(state.begin(), state.begin()+i);
          Subset csub= get_sub_subset(s, order.begin(),
                                      order.begin()+i);
          SubsetState cstate= get_sub_subset_state(s,
                                                   state,
                                                   csub);
          for (unsigned int j=0; j < csub.size(); ++j) {
            IMP_INTERNAL_CHECK(cstate[j] == map.find(csub[j])->second,
                               "Wrong state found in " << cstate
                               << " for " << csub << " from "
                               << s << std::endl);
          }
        }
        {
          //Ints pstate(state.begin()+i, state.end());
          Subset csub= get_sub_subset(s, order.begin()+i,
                                      order.end());
          SubsetState cstate= get_sub_subset_state(s, state, csub);
          for (unsigned int j=0; j < csub.size(); ++j) {
            IMP_INTERNAL_CHECK(cstate[j] == map.find(csub[j])->second,
                               "Wrong state found in back " << cstate
                               << " for " << csub << " from "
                               << s << std::endl);
          }
        }
        }
        }*/

  filter:
    //std::cout << "Filtering " << cur << " on " << changed_digit << std::endl;
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      /*for (unsigned int i=changed_digit+1; i < cur.size(); ++i) {
        Subset subset= get_sub_subset(s, order.begin()+i, order.end());
        SubsetState sub_state= get_sub_subset_state(s, cur, subset);
        IMP_IF_CHECK(USAGE_AND_INTERNAL) {
          IMP_INTERNAL_CHECK(subset== filter_subsets[i],
                             "Expected and found subsets don't match "
                             << filter_subsets[i] << " vs " << subset);
        }
        for (unsigned int j=0; j< filters[i].size(); ++j) {
          IMP_INTERNAL_CHECK(filters[i][j]->get_is_ok(sub_state),
                             "Subset should have been passed before "
                             << get_sub_subset(s, order.begin()+i, order.end())
                             << " with " << sub_state << std::endl);
        }
        }*/
    }
    for (int i=changed_digit; i >=0; --i) {
      for (unsigned int j=0; j < filters[i].size(); ++j) {
        // use boost iterator wrapper TODO
        // check +i is correct
        Subset subset= get_sub_subset(s, order.begin()+i, order.end());
        SubsetState state= get_sub_subset_state(s, cur, subset);
        /*std::cout << "filtering " << i << " " << j
                  << " on state " << state
                  << " got " << filters[i][j]->get_is_ok(state)
                  << std::endl;*/
        IMP_IF_CHECK(USAGE_AND_INTERNAL) {
          IMP_INTERNAL_CHECK(subset== filter_subsets[i],
                             "Expected and found subsets don't match "
                             << filter_subsets[i] << " vs " << subset);
        }
        IMP_CHECK_OBJECT(filters[i][j]);
        if (!filters[i][j]->get_is_ok(state)) {
          goto bad;
        }
      }
      continue;
    bad:
      //std::cout << "Failed at " << i << std::endl;
      current_digit=i;
      goto increment;
    }
    {
      current_digit=0;
      SubsetState to_push(cur);
      IMP_LOG(VERBOSE, "Found " << to_push << std::endl);
      states_.push_back(to_push);
    }
  increment:
    //std::cout << "Incrementing " << cur << " on "
    //<< current_digit << std::endl;
    for (unsigned int i=0; i< current_digit; ++i) {
      cur[order[i]]=0;
    }
    for (unsigned int i=current_digit; i < cur.size(); ++i) {
      ++cur[order[i]];
      if (cur[order[i]]==maxs[order[i]]) {
        cur[order[i]]=0;
      } else {
        changed_digit=i;
        goto filter;
      }
    }
    //done:
    std::sort(states_.begin(), states_.end());
    IMP_LOG(TERSE, "found " << states_.size() << std::endl);
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
  IMP_OBJECT_LOG;
  SubsetStates*ret= new BranchAndBoundSubsetStates(s,
                                                   pst_, sft_);
  ret->set_log_level(get_log_level());
  return ret;
}

void BranchAndBoundSubsetStatesTable::do_show(std::ostream &out) const {
}



// List.....
ListSubsetStates::ListSubsetStates(std::string name): SubsetStates(name) {
}

unsigned int ListSubsetStates::get_number_of_states() const {
  return states_.size();
}
SubsetState ListSubsetStates::get_state(unsigned int i) const {
  return states_[i];
}

bool
ListSubsetStates::get_is_state(const SubsetState &state) const {
  return std::binary_search(states_.begin(), states_.end(), state);
}

void ListSubsetStates::add_subset_state(const SubsetState& s) {
  states_.insert(std::lower_bound(states_.begin(), states_.end(), s), s);
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=1; i< states_.size(); ++i) {
      IMP_USAGE_CHECK(states_[i-1] < states_[i],
                      "Not ordered properly");
    }
  }
}

void ListSubsetStates::do_show(std::ostream &out) const{}


ListSubsetStatesTable
::ListSubsetStatesTable(std::string name): SubsetStatesTable(name) {}


SubsetStates* ListSubsetStatesTable
::get_subset_states(const Subset&s) const {
  return states_.find(s)->second;
}

void ListSubsetStatesTable::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
