/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/base/map.h>
#include <algorithm>
#include <boost/version.hpp>
#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>
#include <boost/iterator/permutation_iterator.hpp>


IMPDOMINO_BEGIN_NAMESPACE
AssignmentsTable::~AssignmentsTable(){}

typedef Ints Ints;

namespace {

  class Counter {
    Ints cur_;
    Ints maxs_;
  public:
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
          IMP_LOG_VERBOSE( "Assignment " << cur << " rejected by "
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
      pac0->set_was_used(true);
      IMP_NEW(PackedAssignmentContainer, pac1, ());
      pac1->set_was_used(true);
      recursive_load_assignments(s0,
                                 pst, sft, max, sat, pac0);
      recursive_load_assignments(s1,
                                 pst, sft, max, sat, pac1);
      IMP_LOG_TERSE( "Merging " << s0
              << "(" << pac0->get_number_of_assignments()
              << ") and " << s1 << "("
              << pac1->get_number_of_assignments() << ")"
              << std::endl);
      internal::EdgeData ed= internal::get_edge_data(s0, s1, sft);
      Ints ui0= internal::get_index(ed.union_subset, s0);
      Ints ui1= internal::get_index(ed.union_subset, s1);
      for (unsigned int i=0; i< pac0->get_number_of_assignments(); ++i) {
        Assignment a0= pac0->get_assignment(i);
        Assignments a1= pac1->get_assignments();
        for (unsigned int j=0; j< a1.size(); ++j) {
          Assignment merged= internal::get_merged_assignment(s, a0, ui0,
                                                             a1[j], ui1);
          bool ok=true;
          for (unsigned int k=0; k< ed.filters.size(); ++k) {
            if (!ed.filters[k]->get_is_ok(merged)) {
              ok=false;
              break;
            }
          }
          if (ok) {
            pac->add_assignment(merged);
          }
        }
      }
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


  typedef IMP::base::map<Particle*, Particle*> IParent;
  typedef IMP::base::map<Particle*, int> IRank;
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


  double evaluate_order(const Ints &order, const Subset &s,
                        const SubsetFilterTables &sft) {
    ParticlesTemp sorted= get_sub_particles(s, order.begin(), order.end());
    Subset sc(sorted);
    sorted.pop_back();
    Subsets excluded;
    if (!sorted.empty()) {
      excluded.push_back(Subset(sorted));
    }
    double ret=1;
    for (unsigned int i=0; i< sft.size(); ++i) {
      double cur= sft[i]->get_strength(sc, excluded);
      IMP_USAGE_CHECK(cur>=0 && cur <=1,
                      "The strength of a filter should be between 0 and 1"
                      << " with 1 being the strongest. It is not for "
                      << Showable(sft[i]) << " at " << cur);
      ret*=1.0-cur;
    }
    return 1.0-ret;
  }

  ParticlesTemp initialize_order(const Subset &s,
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
      IMP_LOG_TERSE( "Order for " << s << " is ");
      ParticlesTemp ps(get_sub_particles(s, order.begin(), order.end()));
      IMP_LOG_TERSE( ps << std::endl);
    }
    return get_sub_particles(s, order.begin(), order.end());
  }

  Assignment get_next_assignment_base(
                                 Ints cur,
                                 const Ints &maxs,
              const base::Vector<SubsetFilters> &filters) {
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
                                     const Subsets &subsets,
                     const base::Vector<Ints>& orders,
                                     Ints cur,
                                     const Ints &maxs,
              const base::Vector<SubsetFilters> &filters) {
    IMP_INTERNAL_CHECK(s.size() == cur.size(), "Subset and last don't match");
    IMP_INTERNAL_CHECK(s.size() == maxs.size(), "Subset and maxs don't match");
    IMP_INTERNAL_CHECK(s.size() == orders.size(),
                       "Subset and orders don't match");
    IMP_INTERNAL_CHECK(s.size() == filters.size(),
                       "Subset and filters don't match");
    unsigned int increment=1;
    while (true) {
      cur.back()+=increment;
      if (cur.back() >= maxs.back()) {
        cur.back()=0;
        Assignment inner;
        if (s.size()==2) {
          inner= get_next_assignment_base(Ints(cur.begin(), cur.end()-1),
                                          Ints(maxs.begin(), maxs.end()-1),
                   base::Vector<SubsetFilters>(filters.begin(),
                                                          filters.end()-1));
        } else {
          inner= get_next_assignment(ParticlesTemp(s.begin(),
                                                   s.end()-1),
                                     Subsets(subsets.begin(),
                                             subsets.end()-1),
                             base::Vector<Ints>(orders.begin(),
                                                       orders.end()-1),
                                     Ints(cur.begin(), cur.end()-1),
                                     Ints(maxs.begin(), maxs.end()-1),
                  base::Vector<SubsetFilters>(filters.begin(),
                                                      filters.end()-1));
        }
        if (inner.size()==0) {
          return inner;
        } else {
          std::copy(inner.begin(), inner.end(), cur.begin());
        }
      }
      Ints reordered_cur(cur.size());
      for (unsigned int i=0; i< orders.back().size(); ++i) {
        reordered_cur[i]= cur[orders.back()[i]];
        IMP_INTERNAL_CHECK(subsets.back()[i]
                           == s[orders.back()[i]],
                           "Ordering doesn't match");
      }
      //std::cout << "Trying " << cur  << " = " << reordered_cur << std::endl;
      Assignment cura(reordered_cur);
      bool ok=true;
      IMP_LOG_VERBOSE( "Trying " << cura << " for "
              << subsets.back() << std::endl);
      for (unsigned int i=0; i< filters.back().size(); ++i) {
        if (!filters.back()[i]->get_is_ok(cura)) {
          unsigned int pos=0;
          for (; pos < cur.size(); ++pos) {
            if (orders.back()[pos]== static_cast<int>(cur.size())-1) {
              break;
            }
          }
          unsigned int next= filters.back()[i]
            ->get_next_state(pos,
                             cura);
          increment= std::max<unsigned int>(next-cur.back(), 1);
          IMP_IF_CHECK(USAGE_AND_INTERNAL) {
            int maxnext
              =std::min<unsigned int>(next, maxs[orders.back()[pos]]);
            IMP_INTERNAL_CHECK(maxnext >=cur.back(), "Confused ordering");
            for (unsigned int j=cura[pos];
                 j < static_cast<unsigned int>(maxnext); ++j) {
              reordered_cur[pos]=j;
              Assignment curat(reordered_cur);
              IMP_INTERNAL_CHECK(!filters.back()[i]->get_is_ok(curat),
                                 "The filter " << filters.back()[i]->get_name()
                                 << " said to skip, but also said it was OK at "
                                 << "position " << pos << " of " << curat
                                 << " skipping until " << cur.back()+increment);
            }
          }
          IMP_INTERNAL_CHECK(cur.back()==cura[pos],
                             "Assignments don't match");
          ok=false;
          IMP_INTERNAL_CHECK(increment > 0, "Increment must be positive,"
                             << " it was not"
                             << " for \"" << filters.back()[i] << "\""
                             << " got " << increment << " from " << cur.back());
          break;
        }
      }
      if (ok) {
        return Assignment(cur);
      }
    }
  }
}


BranchAndBoundAssignmentsTable
::BranchAndBoundAssignmentsTable(ParticleStatesTable *pst,
                                  const SubsetFilterTables &sft,
                                  unsigned int max):
  pst_(pst), sft_(sft), max_(max){
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Created BranchAndBoundAssignments with filters: ");
  IMP_IF_LOG(TERSE) {
    for (unsigned int i=0; i< sft.size(); ++i) {
      IMP_LOG_TERSE( *sft[i] << std::endl);
    }
  }
}




void BranchAndBoundAssignmentsTable
::load_assignments(const Subset&s, AssignmentContainer*pac) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  ParticlesTemp spt=initialize_order(s, sft_);
  //std::reverse(spt.begin(), spt.end());
  Ints cur(s.size(), std::numeric_limits<int>::max()-3); cur.front()=-1;
  Ints maxs(cur.size());
  for (unsigned int i=0; i< maxs.size(); ++i) {
    maxs[i]= pst_->get_particle_states(spt[i])->get_number_of_particle_states();
  }
  base::Vector<SubsetFilters> filters(maxs.size());
  base::Vector<Ints> orders(maxs.size());
  Subsets subsets(maxs.size());
  for (unsigned int i=0; i< maxs.size(); ++i) {
    Subsets excluded;
    if (i>0) {
      excluded.push_back(Subset(ParticlesTemp(spt.begin(), spt.begin()+i)));
    }
    Subset cur(ParticlesTemp(spt.begin(), spt.begin()+i+1));
    Ints order(cur.size());
    for (unsigned int j=0; j< cur.size(); ++j) {
      for (unsigned int k=0;  k < cur.size(); ++k) {
        if (cur[j]== spt[k]) {
          order[j]=k;
        }
      }
    }
    subsets[i]=cur;
    orders[i]=order;
    for (unsigned int j=0; j< sft_.size(); ++j) {
      SubsetFilter *sf= sft_[j]->get_subset_filter(cur, excluded);
      if (sf) {
        filters[i].push_back(sf);
      }
    }
  }
  do {
    Assignment cura;
    if (spt.size()>1) {
      cura=get_next_assignment(spt,
                               subsets,
                               orders,
                               cur,
                               maxs,
                               filters);
    } else {
      cura=get_next_assignment_base(cur,
                               maxs, filters);
    }
    if (cura.size()>0) {
      Ints cura_reordered(cura.size());
      for (unsigned int i=0; i< cura.size(); ++i) {
        cura_reordered[i]= cura[orders.back()[i]];
      }
      /*std::cout << "adding " << cura
                << " = " << Assignment(cura_reordered)
                << std::endl;*/
      IMP_LOG_VERBOSE( "Found " << cura_reordered << std::endl);
      pac->add_assignment(Assignment(cura_reordered));
      std::copy(cura.begin(), cura.end(), cur.begin());
    } else {
      break;
    }
  } while (true);
#if 0
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    int space=1;
    for (unsigned int i=0; i< s.size(); ++i) {
      ParticleStates *ps=pst_->get_particle_states(s[i]);
      IMP_CHECK_OBJECT(ps);
      unsigned int n= ps->get_number_of_particle_states();
      IMP_INTERNAL_CHECK(n>0 && n < 1000000, "Out of range num states"
                         << n);
      space*= n;
      if (space > 10000) {
        break;
      }
    }
    if (space< 10000) {
      IMP_LOG_TERSE( "Verifying output..." << std::endl);
      IMP_NEW(PackedAssignmentContainer, cpac, ());
      cpac->set_was_used(true);
      IMP_NEW(SimpleAssignmentsTable, sat, (pst_, sft_, max_));
      //sat->set_log_level(SILENT);
      sat->load_assignments(s, cpac);
      using namespace IMP;
      if (cpac->get_number_of_assignments()
          != pac->get_number_of_assignments()) {
        Assignments a0= cpac->get_assignments();
        Assignments a1= pac->get_assignments();
        std::sort(a0.begin(), a0.end());
        std::sort(a1.begin(), a1.end());
        Assignments diff;
        std::set_symmetric_difference(a0.begin(), a0.end(),
                                      a1.begin(), a1.end(),
                            std::back_inserter(diff));
        IMP_LOG(WARNING, a0
                << "\n vs \n"
                << a1
                << "\n diff\n" << diff);
        IMP_INTERNAL_CHECK(0,
                           "Numbers don't match "
                           << cpac->get_number_of_assignments()
                           << " vs " << pac->get_number_of_assignments()
                           << " for " << s
                           << "\n");
      }
      Assignments paca= pac->get_assignments();
      Assignments cpaca= cpac->get_assignments();
      std::sort(paca.begin(), paca.end());
      std::sort(cpaca.begin(), cpaca.end());
      for (unsigned int i=0; i< paca.size(); ++i) {
        IMP_INTERNAL_CHECK(paca[i]==cpaca[i], "Don't match.");
      }
    }
  }
#endif
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


ParticlesTemp get_order(const Subset &s,
               const SubsetFilterTables &sft) {
  ParticlesTemp order=initialize_order(s, sft);
  return order;
}


IMPDOMINO_END_NAMESPACE
