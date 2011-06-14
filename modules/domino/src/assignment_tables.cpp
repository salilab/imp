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
#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <boost/iterator/permutation_iterator.hpp>


IMPDOMINO_BEGIN_NAMESPACE
AssignmentsTable::~AssignmentsTable(){}

typedef std::vector<int> Ints;

namespace {
  class Counter {
    Ints cur_;
    Ints maxs_;
  public:
    Counter(){}
    Counter(const Ints &maxs): cur_(maxs.size(), 0),
                                maxs_(maxs) {}
    unsigned int increment(unsigned int digit, unsigned int amount) {
      IMP_USAGE_CHECK(digit < cur_.size(), "Out of range digit");
      for (unsigned int i=0; i< digit; ++i) {
        cur_[i]=0;
      }
      for (unsigned int i=digit; i < cur_.size() && amount>0; ++i) {
        cur_[i]+=amount;
        if (cur_[i] >= maxs_[i]) {
          cur_[i]=0;
          amount=1;
        } else {
          return i;
        }
      }
      return cur_.size();
    }
    const Ints& get_counter() const {
      return cur_;
    }
  };


  void brute_load_assignments(const Subset &s,
                              ParticleStatesTable *pst,
                              const SubsetFilterTables &sft,
                              unsigned int max,
                              AssignmentContainer *pac) {
    Ints maxs(s.size());
    for (unsigned int i=0; i< maxs.size(); ++i) {
      maxs[i]= pst->get_particle_states(s[i])->get_number_of_particle_states();
    }
    SubsetFilters sfs;
    for (unsigned int i=0; i< sft.size(); ++i) {
      SubsetFilter *cur= sft[i]->get_subset_filter(s, Subsets());
      if (cur) {
        sfs.push_back(cur);
      }
    }
    Counter count(maxs);
    do {
      Assignment cur(count.get_counter());
      bool ok=true;
      for (unsigned int i=0; i<  sfs.size(); ++i) {
        if (!sfs[i]->get_is_ok(cur)) {
          IMP_LOG(VERBOSE, "Assignment " << cur << " rejected by "
                  << sfs[i]->get_name()
                  << std::endl);
          ok=false;
          break;
        }
      }
      if (ok) {
        pac->add_assignment(cur);
        if (pac->get_number_of_assignments() > max) {
          IMP_WARN("Filled up quota on subset " << s);
          return;
        }
      }
    } while (count.increment(0,1) < maxs.size());
  }

  void recursive_load_assignments(const Subset &s,
                                  ParticleStatesTable *pst,
                                  const SubsetFilterTables &sft,
                                  int max, SimpleAssignmentsTable *sat,
                                  AssignmentContainer *pac) {
    if (s.size() <=2) {
      sat->load_assignments(s, pac);
    } else {
      int mp= s.size()/2;
      Subset s0(ParticlesTemp(s.begin(), s.begin()+mp));
      Subset s1(ParticlesTemp(s.begin()+mp, s.end()));
      IMP_NEW(PackedAssignmentContainer, pac0, ());
      IMP_NEW(PackedAssignmentContainer, pac1, ());
      recursive_load_assignments(s0,
                                 pst, sft, max, sat, pac0);
      recursive_load_assignments(s1,
                                 pst, sft, max, sat, pac1);
      internal::EdgeData ed= internal::get_edge_data(s0, s1, sft);
      internal::load_union(s0, s1, pac0, pac1, ed, max, pac);
    }
  }

}

SimpleAssignmentsTable::SimpleAssignmentsTable(ParticleStatesTable* pst,
                                               const SubsetFilterTables &sft,
                                               unsigned int max):
  pst_(pst), sft_(sft), max_(max){}




void SimpleAssignmentsTable
::load_assignments(const Subset&s, AssignmentContainer*pac) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  brute_load_assignments(s, pst_, sft_, max_, pac);
}

void SimpleAssignmentsTable::do_show(std::ostream &) const {
}


RecursiveAssignmentsTable::RecursiveAssignmentsTable(ParticleStatesTable* pst,
                                               const SubsetFilterTables &sft,
                                               unsigned int max):
  pst_(pst), sft_(sft), max_(max){}




void RecursiveAssignmentsTable
::load_assignments(const Subset&s, AssignmentContainer*pac) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_NEW(SimpleAssignmentsTable, sac, (pst_, sft_, max_));
  recursive_load_assignments(s, pst_, sft_, max_, sac, pac);
}

void RecursiveAssignmentsTable::do_show(std::ostream &) const {
}

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
    return rs;
  }
  template <class Ints>
  Assignment get_sub_assignment(const Subset &s, const Ints &ss,
                                   const Subset sub_s) {
    Ints ret(sub_s.size());
    for (unsigned int i=0; i< sub_s.size(); ++i) {
      for (unsigned int j=0; j< s.size(); ++j) {
        if (sub_s[i] == s[j]) {
          ret[i]= ss[j];
        }
      }
    }
    return Assignment(ret);
  }


  SubsetFilters get_filters(const Subset &sc,
                            const Subsets &excluded,
                            const SubsetFilterTables &sft) {
    SubsetFilters ret;
    for (unsigned int i=0; i< sft.size(); ++i) {
      SubsetFilter* f= sft[i]->get_subset_filter(sc, excluded);
      if (f) {
        ret.push_back(f);
        ret.back()->set_was_used(true);
      } else {
      }
    }
    return ret;
  }


  double evaluate_order(const Ints &order, const Subset &s,
                        const SubsetFilterTables &sft) {
    ParticlesTemp sorted= get_sub_particles(s, order.begin(), order.end());
    Subset sc(sorted);
    sorted.pop_back();
    Subsets excluded;
    if (!sorted.empty()) {
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
    IMP_FUNCTION_LOG;
    Ints order;
    Ints remaining;
    for (unsigned int i=0; i< s.size(); ++i) {
      remaining.push_back(i);
    }
    while (order.size() != s.size()) {
      double max_restraint=-1;
      int max_j=-1;
      for (unsigned int j=0; j< remaining.size(); ++j) {
        //std::cout << "Trying " << remaining[j] << std::endl;
        int cur= remaining[j];
        order.push_back(cur);

        // ask all tables about subset of taken+this particle - taken
        double cur_restraint=evaluate_order(order, s, sft);

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

  Assignment get_next_assignment_base(const ParticlesTemp &,
                                 Ints cur,
                                 const Ints &maxs,
                                 const std::vector<SubsetFilters> &filters) {
    unsigned int increment=1;
    while (true) {
      cur[0]+=increment;
      if (cur[0] >= maxs[0]) {
        return Assignment();
      }
      Assignment cura(cur);
      bool ok=true;
      for (unsigned int i=0; i< filters.back().size(); ++i) {
        if (!filters.back()[i]->get_is_ok(cura)) {
          increment= filters.back()[i]->get_next_state(0, cura)- cur[0];
          ok=false;
          break;
        }
      }
      if (ok) {
        return cura;
      }
    }
  }

  Assignment get_next_assignment(const ParticlesTemp &s,
                                 Ints cur,
                                 const Ints &maxs,
                                const std::vector<SubsetFilters> &filters);

  Assignment get_next_assignment_add(const ParticlesTemp &s,
                                     Ints cur,
                                     const Ints &maxs,
                                  const std::vector<SubsetFilters> &filters) {
    unsigned int increment=1;
    while (true) {
      Assignment inner= get_next_assignment(ParticlesTemp(s.begin(), s.end()-1),
                                            Ints(cur.begin(), cur.end()-1),
                                            Ints(maxs.begin(), maxs.end()-1),
                      std::vector<SubsetFilters>(filters.begin(),
                                                 filters.end()-1));
      if (inner.size()==0) {
        // done
        cur.back()+=increment;
        if (cur.back() >= maxs.back()) {
          return Assignment();
        }
        std::fill(cur.begin(), cur.begin()+cur.size()-1, 0);
        cur[0]=-1;
        // do the inner increment again
        continue;
      } else {
        std::copy(inner.begin(), inner.end(), cur.begin());
      }
      Assignment cura(cur);
      bool ok=true;
      for (unsigned int i=0; i< filters.back().size(); ++i) {
        if (!filters.back()[i]->get_is_ok(cura)) {
          increment= filters.back()[i]
            ->get_next_state(cur.size()-1, cura)- cur.back();
          ok=false;
          break;
        }
      }
      if (ok) {
        return cura;
      }
    }
  }


  Assignment get_next_assignment(const ParticlesTemp &s,
                                 Ints cur,
                                 const Ints &maxs,
                                 const std::vector<SubsetFilters> &filters) {
    IMP_INTERNAL_CHECK(s.size() == cur.size(), "Subset and last don't match");
    IMP_INTERNAL_CHECK(s.size() == maxs.size(), "Subset and maxs don't match");
    IMP_INTERNAL_CHECK(s.size() == filters.size(),
                       "Subset and filters don't match");
    if (s.size()==1) {
      return get_next_assignment_base(s, cur, maxs, filters);
    } else {
      return get_next_assignment_add(s, cur, maxs, filters);
    }
  }
}
#if 0

  int do_increment(Ints &cur,
                   const Ints &order,
                   unsigned int current_digit,
                   const Ints &maxs,
                   unsigned int incr) {
    for (unsigned int i=0; i< current_digit; ++i) {
      cur[order[i]]=0;
    }
    for (unsigned int i=current_digit; i < cur.size(); ++i) {
      cur[order[i]]+=incr;
      // just use the increment of 1 for later states
      incr=1;
      if (cur[order[i]]>=maxs[order[i]]) {
        cur[order[i]]=0;
      } else {
        return i;
      }
    }
    return cur.size();
  }

  unsigned int add_assignment(const Ints &cur,
                              const Subset &s,
                              AssignmentContainer *states) {
    unsigned int sz= states->get_number_of_assignments();
    Assignment to_push(cur);
    IMP_LOG(VERBOSE, "Found " << to_push << std::endl);
    try {
      states->add_assignment(to_push);
    } catch (std::bad_alloc) {
      IMP_THROW("Ran out of memory when enumerating states for " << s
                << " with " << sz << " states found so far."
                << " Last state is " << to_push, ValueException);
    }
    return states->get_number_of_assignments();
  }

  std::vector<SubsetFilters> create_filters(const SubsetFilterTables &sft,
                                            const Ints& order,
                                            const Subset &s,
                                            Subsets &filter_subsets) {
    std::vector<SubsetFilters> filters;
    for (unsigned int i=0; i< order.size(); ++i) {
      ParticlesTemp ps= get_sub_particles(s, order.begin()+i, order.end());
      Subset sc(ps);
      Subsets ex;
      ParticlesTemp pt(ps.begin()+1, ps.end());
      if (!pt.empty()) {
        ex.push_back(Subset(pt));
      }
      SubsetFilters fts= get_filters(sc, ex, sft);
      filter_subsets.push_back(sc);
      filters.push_back(fts);
    }
    return filters;
  }

  int do_filter(unsigned int i, const std::vector<SubsetFilters> &filters,
                const Subsets &filter_subsets,
                const Subset &s, const Ints &order, const Ints &cur,
                const IMP::internal::Map<Particle*, ParticlesTemp> rls) {
    Subset subset= get_sub_subset(s, order.begin()+i, order.end());
    Assignment state= get_sub_assignment(s, cur, subset);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      IMP_INTERNAL_CHECK(subset== filter_subsets[i],
                         "Expected and found subsets don't match "
                         << filter_subsets[i] << " vs " << subset);
    }
    for (unsigned int j=0; j < filters[i].size(); ++j) {
      // use boost iterator wrapper TODO
      // check +i is correct
      IMP_CHECK_OBJECT(filters[i][j]);
      if (!filters[i][j]->get_is_ok(state)) {
        IMP_LOG(VERBOSE, "Rejected state " << state
                << " on prefix subset " << subset << " due to filter "
                << *filters[i][j] << std::endl);
        int pos=-1;
        for (unsigned int k=0; k< subset.size(); ++k) {
          if (subset[k]== s[order[i]]) {
            pos=k;
            break;
          }
        }
        IMP_USAGE_CHECK(pos != -1, "Particle not found " << s << " vs "
                        << subset << " " << s[order[i]]->get_name());
        int ret= filters[i][j]->get_next_state(pos, state)- state[pos];
        IMP_LOG(VERBOSE, "Next state for " <<  filters[i][j]->get_name()
                << " is " << ret+state[pos] << " from " << state[pos]
                << " in " << state << " on " << subset << std::endl);
        IMP_USAGE_CHECK(ret>0, "invalid next state returned by "
                        << filters[i][j]->get_name()
                        << " " << filters[i][j]->get_next_state(pos, state));
        return ret;
      }
    }
    return 0;
  }

  void load_states_list(const Subset &s,
                        ParticleStatesTable *table,
                        const SubsetFilterTables &sft,
                        unsigned int max,
                        const IMP::internal::Map<Particle*, ParticlesTemp> rls,
                        AssignmentContainer *states) {
    //std::cout << "Searching order for " << s << std::endl;

    IMP_FUNCTION_LOG;
    Ints order=initialize_order(s, sft);
    std::reverse(order.begin(), order.end());
    std::vector<Subset> filter_subsets;
    std::vector<SubsetFilters> filters=create_filters(sft, order, s,
                                                      filter_subsets);
    int incr=1;

    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      IMP::internal::Set<int> taken(order.begin(), order.end());
      IMP_INTERNAL_CHECK(taken.size() == order.size(),
                         "Duplicate elements in order "
                         << taken.size() << " " << order.size());
    }
    IMP_LOG(TERSE, "Enumerating states for " << s << "..." << std::endl);

    IMP_CHECK_OBJECT(table);
    // create lists

    unsigned int sz=s.size();
    Ints maxs(sz);
    for (unsigned int i=0; i< sz; ++i) {
      maxs[i]=table->get_particle_states(s[i])
        ->get_number_of_particle_states();
    }
    Ints cur(sz, 0);
    unsigned int changed_digit=cur.size()-1;
    unsigned int current_digit=0;

  filter:
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "Current counter is ");
      for (unsigned int i=0; i< order.size(); ++i) {
        IMP_LOG(VERBOSE, cur[order[i]] << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    // reset the increment
    for (int i=changed_digit; i >=0; --i) {
      incr=do_filter(i, filters, filter_subsets,
                     s, order, cur, rls);
      if (incr > 0) {
        current_digit=i;
        break;
      }
    }
    if (incr==0) {
      //current_digit=0;
      incr=1;
      if (add_assignment(cur, s, states) ==max) {
              IMP_WARN("Truncated subset states at " << max
               << " for subset " << s);
              goto done;
      }
    }
    changed_digit=do_increment(cur, order, current_digit, maxs, incr);
    if (changed_digit < order.size()) {
      goto filter;
    }
  done:
    IMP_IF_LOG(TERSE) {
      std::size_t possible=1;
      for (unsigned int i=0; i< s.size(); ++i) {
        Pointer<ParticleStates> ps= table->get_particle_states(s[i]);
        possible= possible*ps->get_number_of_particle_states();
      }
      IMP_LOG(TERSE, "In total found " << states->get_number_of_assignments()
              << " for subset " << s << " of " << possible
              << " possible states." << std::endl);
    }
  }
}
#endif

BranchAndBoundAssignmentsTable
::BranchAndBoundAssignmentsTable(ParticleStatesTable *pst,
                                  const SubsetFilterTables &sft,
                                  unsigned int max):
  pst_(pst), sft_(sft), max_(max){
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Created BranchAndBoundAssignments with filters: ");
  IMP_IF_LOG(TERSE) {
    for (unsigned int i=0; i< sft.size(); ++i) {
      IMP_LOG(TERSE, *sft[i] << std::endl);
    }
  }
  /*
#if IMP_BUILD < IMP_FAST
  ParticlesTemp ps=pst->get_particles();
  if (!ps.empty()) {
    Model*m= ps[0]->get_model();
    DependencyGraph dg
      = get_dependency_graph(RestraintsTemp(1,
                                            m->get_root_restraint_set()));
    const ParticlesTemp all= pst->get_particles();
    for (unsigned int i=0; i < all.size(); ++i) {
      Particle *p= all[i];
      ParticlesTemp ps= get_dependent_particles(p, all, dg);
      for (unsigned int j=0; j< ps.size(); ++j) {
        rls_[p]=ps;
      }
    }
  }
  #endif*/
}


void BranchAndBoundAssignmentsTable
::load_assignments(const Subset&s, AssignmentContainer*pac) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  ParticlesTemp spt(s.begin(), s.end());
  Ints cur(s.size(), 0); cur.front()=-1;
  Ints maxs(cur.size());
  for (unsigned int i=0; i< maxs.size(); ++i) {
    maxs[i]= pst_->get_particle_states(s[i])->get_number_of_particle_states();
  }
  std::vector<SubsetFilters> filters(maxs.size());
  for (unsigned int i=0; i< maxs.size(); ++i) {
    Subsets excluded;
    if (i>0) {
      excluded.push_back(Subset(ParticlesTemp(s.begin(), s.begin()+i)));
    }
    Subset cur(ParticlesTemp(s.begin(), s.begin()+i+1));
    for (unsigned int j=0; j< sft_.size(); ++j) {
      SubsetFilter *sf= sft_[j]->get_subset_filter(cur, excluded);
      if (sf) {
        filters[i].push_back(sf);
      }
    }
  }
  do {
    Assignment cura= get_next_assignment(spt,
                                        cur,
                                        maxs,
                                        filters);
    if (cura.size()>0) {
      std::cout << "adding " << cura << std::endl;
      pac->add_assignment(cura);
      std::copy(cura.begin(), cura.end(), cur.begin());
    } else {
      break;
    }
  } while (true);

  if (pac->get_number_of_assignments()< 10000) {
    IMP_NEW(PackedAssignmentContainer, cpac, ());
    IMP_NEW(RecursiveAssignmentsTable, sat, (pst_, sft_, max_));
    sat->load_assignments(s, cpac);
    IMP_INTERNAL_CHECK(cpac->get_number_of_assignments()
                       == pac->get_number_of_assignments(),
                    "Numbers don't match " << cpac->get_number_of_assignments()
                    << " vs " << pac->get_number_of_assignments());
    Assignments paca= pac->get_assignments();
    Assignments cpaca= cpac->get_assignments();
    std::sort(paca.begin(), paca.end());
    std::sort(cpaca.begin(), cpaca.end());
    for (unsigned int i=0; i< paca.size(); ++i) {
      IMP_INTERNAL_CHECK(paca[i]==cpaca[i], "Don't match.");
    }
  }
}

void BranchAndBoundAssignmentsTable::do_show(std::ostream &) const {
}


ListAssignmentsTable
::ListAssignmentsTable(std::string name): AssignmentsTable(name) {}

void ListAssignmentsTable
::load_assignments(const Subset &s,
                   AssignmentContainer *out) const {
  set_was_used(true);
  IMP_USAGE_CHECK(states_.find(s) != states_.end(),
                  "I don't know anything about subset " << s);
  out->add_assignments(states_.find(s)->second->get_assignments(IntRange(0,
                   states_.find(s)->second->get_number_of_assignments())));
}


void ListAssignmentsTable::do_show(std::ostream &) const {
}


Ints get_order(const Subset &s,
               const SubsetFilterTables &sft) {
  Ints order=initialize_order(s, sft);
  return order;
}


IMPDOMINO_END_NAMESPACE
