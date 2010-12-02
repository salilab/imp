/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/internal/map.h>
#include <set>
#include <boost/version.hpp>
#include <IMP/domino/subset_states.h>
#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <boost/iterator/permutation_iterator.hpp>


IMPDOMINO_BEGIN_NAMESPACE
SubsetStatesTable::~SubsetStatesTable(){}

typedef std::vector<int> Ints;

namespace {
  typedef IMP::internal::Map<Particle*, Particle*> IParent;
  typedef IMP::internal::Map<Particle*, int> IRank;
  typedef boost::associative_property_map<IParent> Parent;
  typedef boost::associative_property_map<IRank > Rank;
  typedef boost::disjoint_sets<Rank, Parent> UF;





  namespace {


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


void setup_filters(const Subset &s,
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
              //std::cout << "strength is " << str << std::endl;
              cur_restraint*= 1-str;
              cur_filters.push_back(cur_filter);
            }
          }
          /*std::cout << "Of " << s[remaining[j]]->get_name()
                    << " plus " << excluded << " got strength " << cur_restraint
                    << std::endl;*/
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
  void fill_states_list(const Subset &s,
                        ParticleStatesTable *table,
                        const SubsetFilterTables &sft,
                        SubsetStates &states_) {
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
    IMP_FUNCTION_LOG;
    IMP_LOG(TERSE, "Enumerating states for " << s << "..." << std::flush);

    IMP_CHECK_OBJECT(table);
    // create lists

    unsigned int sz=s.size();
    Ints maxs(sz);
    for (unsigned int i=0; i< sz; ++i) {
      maxs[i]=table->get_particle_states(s[i])
        ->get_number_of_particle_states();
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
          IMP_LOG(VERBOSE, "Rejected state " << state
                  << " on prefix subset " << subset << " due to filter "
                  << *filters[i][j] << std::endl);
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
      unsigned int sz= states_.size();
      SubsetState to_push(cur);
      IMP_LOG(VERBOSE, "Found " << to_push << std::endl);
      try {
        states_.push_back(to_push);
      } catch (std::bad_alloc) {
        IMP_THROW("Ran out of memory when enumerating states for " << s
                  << " with " << sz << " states found so far."
                  << " Last state is " << to_push, ValueException);
      }
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
    IMP_IF_LOG(TERSE) {
      std::size_t possible=1;
      for (unsigned int i=0; i< s.size(); ++i) {
        Pointer<ParticleStates> ps= table->get_particle_states(s[i]);
        possible= possible*ps->get_number_of_particle_states();
      }
      IMP_LOG(TERSE, "In total found " << states_.size()
              << " for subset " << s << " of " << possible
              << " possible states." << std::endl);
    }
  }

}


BranchAndBoundSubsetStatesTable
::BranchAndBoundSubsetStatesTable(ParticleStatesTable *pst,
                                  const SubsetFilterTables &sft):
  pst_(pst), sft_(sft){
  IMP_LOG(TERSE, "Created BranchAndBoundSubsetStates with filters: ");
  IMP_IF_LOG(TERSE) {
    for (unsigned int i=0; i< sft.size(); ++i) {
      IMP_LOG(TERSE, *sft[i] << std::endl);
    }
  }
}


SubsetStates BranchAndBoundSubsetStatesTable
::get_subset_states(const Subset&s) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  SubsetStates ssl;
  fill_states_list(s, pst_, sft_, ssl);
  return ssl;
}

void BranchAndBoundSubsetStatesTable::do_show(std::ostream &out) const {
}


ListSubsetStatesTable
::ListSubsetStatesTable(std::string name): SubsetStatesTable(name) {}

SubsetStates ListSubsetStatesTable
::get_subset_states(const Subset &s) const {
  set_was_used(true);
  IMP_USAGE_CHECK(states_.find(s) != states_.end(),
                  "I don't know anything about subset " << s);
  return states_.find(s)->second;
}


void ListSubsetStatesTable::do_show(std::ostream &out) const {
}

IMPDOMINO_END_NAMESPACE
