/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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



  template <class It>
  ParticlesTemp get_sub_particles(const Subset &s, It b, It e) {
    if (b==e) return ParticlesTemp();
    return ParticlesTemp(boost::make_permutation_iterator(s.begin(), b),
                         boost::make_permutation_iterator(s.end(), e));
  }


  template <class It>
  Subset get_sub_subset(const Subset &s, It b, It e) {
    ParticlesTemp pt= get_sub_particles(s, b, e);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (int i=0; i< std::distance(b,e); ++i) {
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


  SubsetFilters get_filters(const Subset &sc,
                            const Subsets &excluded,
                            const SubsetFilterTables &sft) {
    SubsetFilters ret;
    for (unsigned int i=0; i< sft.size(); ++i) {
      SubsetFilter* f= sft[i]->get_subset_filter(sc, excluded);
      if (f) {
        ret.push_back(f);
      } else {
        /*std::cout << "No filter for " << sft[i]->get_name()
                  << " on " << sc
                  << " minus " << (excluded.empty()?Subset(): excluded[0])
                  << std::endl;*/
      }
    }
    return ret;
  }


  double evaluate_order(const Ints &order, const Subset &s,
                        const SubsetFilterTables &sft) {
    ParticlesTemp sorted= get_sub_particles(s, order.begin(), order.end());
    Subset sc(sorted);
    //std::cout << "sc is " << sc << std::endl;
    sorted.pop_back();
    Subsets excluded;
    if (!sorted.empty()) {
      //std::cout << "excluded is " << Subset(sorted) << std::endl;
      excluded.push_back(Subset(sorted));
    }
    double ret=0;
    for (unsigned int i=0; i< sft.size(); ++i) {
      ret+= sft[i]->get_strength(sc, excluded);
    }
    return ret;
  }

  Ints initialize_order(const Subset &s,
                        const SubsetFilterTables &sft) {
    Ints order;
    Ints remaining;
    for (unsigned int i=0; i< s.size(); ++i) {
      remaining.push_back(i);
    }
    while (order.size() != s.size()) {
      double max_restraint=-1;
      int max_j=-1;
      /*std::cout << "Cur order is ";
      for (unsigned int i=0; i< order.size(); ++i) {
        std::cout << order[i] << " ";
      }
      std::cout << std::endl;*/
      for (unsigned int j=0; j< remaining.size(); ++j) {
        //std::cout << "Trying " << remaining[j] << std::endl;
        int cur= remaining[j];
        order.push_back(cur);

        // ask all tables about subset of taken+this particle - taken
        double cur_restraint=evaluate_order(order, s, sft);

        /*std::cout << "Of " << s[remaining[j]]->get_name()
          << " plus " << excluded << " got strength "
          << cur_restraint
          << std::endl;*/
        /*std::cout << "For " << remaining[j] << " got "
          << cur_restraint << std::endl;*/
        if (cur_restraint >= max_restraint) {
          max_restraint=cur_restraint;
          max_j=j;
        }
        order.pop_back();
      }
      order.push_back(remaining[max_j]);
      remaining.erase(remaining.begin()+max_j);
    }
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE, "Order for " << s << " is ");
      Particles ps(get_sub_particles(s, order.begin(), order.end()));
      IMP_LOG(TERSE, ps << std::endl);
    }
    return order;
  }
  void fill_states_list(const Subset &s,
                        ParticleStatesTable *table,
                        const SubsetFilterTables &sft,
                        unsigned int max,
                        SubsetStates &states_) {
    //std::cout << "Searching order for " << s << std::endl;

    IMP_FUNCTION_LOG;
    Ints order=initialize_order(s, sft);
    std::reverse(order.begin(), order.end());
    std::vector<SubsetFilters> filters;
    std::vector<Subset> filter_subsets;
    int incr=1;
    for (unsigned int i=0; i< order.size(); ++i) {
      ParticlesTemp ps= get_sub_particles(s, order.begin()+i, order.end());
      Subset sc(ps);
      Subset ex(ParticlesTemp(ps.begin()+1, ps.end()));
      SubsetFilters fts= get_filters(sc, Subsets(1, ex), sft);
      filter_subsets.push_back(sc);
      filters.push_back(fts);
    }

    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      std::set<int> taken(order.begin(), order.end());
      IMP_INTERNAL_CHECK(taken.size() == order.size(),
                         "Duplicate elements in order "
                         << taken.size() << " " << order.size());
    }
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
          incr= filters[i][j]->get_next_state(order[i], state)- state[order[i]];
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
      if (states_.size() > max) {
        IMP_WARN("Truncated subset states at " << max
                 << " for subset " << s);
        goto done;
      }
    }
  increment:
    //std::cout << "Incrementing " << cur << " on "
    //<< current_digit << std::endl;
    for (unsigned int i=0; i< current_digit; ++i) {
      cur[order[i]]=0;
    }
    for (unsigned int i=current_digit; i < cur.size(); ++i) {
      cur[order[i]]+=incr;
      if (cur[order[i]]>=maxs[order[i]]) {
        cur[order[i]]=0;
      } else {
        changed_digit=i;
        goto filter;
      }
    }
  done:
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
                                  const SubsetFilterTables &sft,
                                  unsigned int max):
  pst_(pst), sft_(sft), max_(max){
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
  fill_states_list(s, pst_, sft_, max_, ssl);
  return ssl;
}

void BranchAndBoundSubsetStatesTable::do_show(std::ostream &) const {
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


void ListSubsetStatesTable::do_show(std::ostream &) const {
}


Ints get_order(const Subset &s,
               const SubsetFilterTables &sft) {
  Ints order=initialize_order(s, sft);
  return order;
}


IMPDOMINO_END_NAMESPACE
