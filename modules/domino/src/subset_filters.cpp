/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/DominoSampler.h>
#include <map>
#include <set>
#include <IMP/domino/subset_states.h>
#include <IMP/domino/particle_states.h>
#include <IMP/core/XYZ.h>



IMPDOMINO_BEGIN_NAMESPACE
SubsetFilter::SubsetFilter(std::string name): Object(name){}
SubsetFilter::~SubsetFilter(){}
SubsetFilterTable::~SubsetFilterTable(){}



/***************************SCORE ******************************/

  bool RestraintScoreSubsetFilter::get_is_ok(const SubsetState &state) const{
    /*IMP_IF_CHECK(USAGE) {
      IMP_USAGE_CHECK(called_.find(state)
                      == called_.end(),
                      "Already called with " << state << " for "
                      << data_.get_subset());
#if IMP_BUILD < IMP_FAST
      called_.insert(state);
#endif
}*/
    IMP_OBJECT_LOG;
    set_was_used(true);
    const bool ok=data_.get_is_ok(state, max_);
    IMP_LOG(VERBOSE, "For subset " << data_.get_subset()
            << (ok?" accepted":" rejected")
            << " state " << state << std::endl);
    return ok;
  }

  void RestraintScoreSubsetFilter::do_show(std::ostream &out) const{}

double RestraintScoreSubsetFilter::get_strength() const {
  return 1-std::pow(.5, static_cast<int>(data_.get_number_of_restraints()));
}

RestraintScoreSubsetFilterTable::StatsPrinter::~StatsPrinter() {
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Resraint filtration statistics (attempts, passes):\n");
    for (unsigned int i=0; i< get()->rdata_.size(); ++i) {
      std::pair<int,int> stat= get()->rdata_[i].get_statistics();
      if (stat.first >0) {
        IMP_LOG(TERSE, "  \""
                << get()->rdata_[i].get_restraint()->get_name()
                << "\" " << stat.first << " " << stat.second << std::endl);
      }
    }
  }
}



RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(RestraintSet *eval,
                                  ParticleStatesTable *pst):
  SubsetFilterTable("RestraintScoreSubsetFilterTable%1%"),
  mset_(new internal::ModelData(eval, pst))
{
}

RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(Model *m,
                                  ParticleStatesTable *pst):
  SubsetFilterTable("RestraintScoreSubsetFilterTable%1%"),
  mset_(new internal::ModelData(m->get_root_restraint_set(), pst))
{
}

SubsetFilter*
RestraintScoreSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Looking for restraints acting on " << s << " minus ");
    for (unsigned int i=0; i< excluded.size(); ++i) {
      IMP_LOG(VERBOSE, excluded[i] << " ");
    }
  }
  // if there are no restraints just here, the total score can't change
  if (mset_->get_subset_data(s, excluded)
      .get_number_of_total_restraints() ==0) {
    IMP_LOG(VERBOSE, "none found" << std::endl);
    return NULL;
  } else {
    IMP_LOG(VERBOSE, mset_->get_subset_data(s, excluded)
            .get_number_of_total_restraints() << " found" << std::endl);
    SubsetFilter* ret
      = new RestraintScoreSubsetFilter(mset_,
                                       mset_->get_subset_data(s,
                                                              excluded),
                                       mset_->get_model()
                                       ->get_maximum_score());
    ret->set_log_level(get_log_level());
    return ret;
  }
}

void RestraintScoreSubsetFilterTable
::add_score(Restraint *r, const Subset &subset,
               const SubsetState &state, double score) {
  mset_->add_score(r, subset, state, score);
}

void RestraintScoreSubsetFilterTable::do_show(std::ostream &out) const {
}



// ******************************* Disjoint sets ********************

namespace {
  void logit(LogLevel l, const Ints &is) {
    IMP_IF_LOG(l) {
      IMP_LOG(l, "[");
      for (unsigned int i=0; i< is.size(); ++i) {
        IMP_LOG(l, is[i] << " ");
      }
      IMP_LOG(l, "] ");
    }
  }
  void logit(LogLevel l, const ParticlesTemp &is) {
    IMP_IF_LOG(l) {
      IMP_LOG(l, "[");
      for (unsigned int i=0; i< is.size(); ++i) {
        IMP_LOG(l, is[i]->get_name() << " ");
      }
      IMP_LOG(l, "] ");
    }
  }


template <class Filter>
class  DisjointSetsSubsetFilter: public SubsetFilter {
  std::vector<Ints> sets_;
public:
  DisjointSetsSubsetFilter(const std::vector<Ints> &sets): sets_(sets) {
    IMP_LOG(TERSE, "Created filter with ");
    IMP_IF_LOG(TERSE) {
      for (unsigned int i=0; i < sets.size(); ++i) {
        logit(TERSE, sets_[i]);
      }
      IMP_LOG(TERSE, std::endl);
    }
  }
  IMP_OBJECT(DisjointSetsSubsetFilter);
  double get_strength() const {
    double r=1;
    for (unsigned int i=0; i< sets_.size(); ++i) {
      r*= std::pow(.1, static_cast<double>(sets_[i].size()));
    }
    return 1-r;
  }
  bool get_is_ok(const SubsetState &state) const{
    IMP_OBJECT_LOG;
    set_was_used(true);
    Filter f;
    for (unsigned int i=0; i< sets_.size(); ++i) {
      if (!f(state, sets_[i])) return false;
    }
    return true;
  }
};
template <class Filter>
void  DisjointSetsSubsetFilter<Filter>::do_show(std::ostream &out) const{}


}

int DisjointSetsSubsetFilterTable::get_index(Particle *p) {
  if (index_.find(p) == index_.end()) {
    index_[p]= elements_.size();
    disjoint_sets_.make_set(elements_.size());
    elements_.push_back(p);
  }
  return index_[p];
}

void DisjointSetsSubsetFilterTable::build_sets() const {
    if (!sets_.empty()) return;
    if (pst_) {
      IMP::internal::Map<ParticleStates*, int> map;
      ParticlesTemp allps= pst_->get_particles();
      std::vector<ParticlesTemp> allsets;
      for (unsigned int i=0; i< allps.size(); ++i) {
        ParticleStates *ps=pst_->get_particle_states(allps[i]);
        if (map.find(ps) == map.end()){
          map[pst_->get_particle_states(allps[i])] = allsets.size();
          allsets.push_back(ParticlesTemp());
        }
        allsets[map.find(ps)->second].push_back(allps[i]);
      }
      for (unsigned int i=0; i< allsets.size(); ++i) {
        if (allsets[i].size()>1) {
          sets_.push_back(allsets[i]);
        }
      }
    }

    std::vector<ParticlesTemp> all(elements_.size());
    for (unsigned int i=0; i< elements_.size(); ++i) {
      int set= disjoint_sets_.find_set(i);
      all[set].push_back(elements_[i]);
    }
    for (unsigned int i=0; i< all.size(); ++i) {
      if (all[i].size() >1 ) sets_.push_back(all[i]);
    }
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE, "Sets are:\n");
      for (unsigned int i=0; i< sets_.size(); ++i) {
        logit(TERSE, sets_[i]);
        IMP_LOG(TERSE, std::endl);
      }
    }
}


  void DisjointSetsSubsetFilterTable::add_set(const ParticlesTemp &ps) {
    if (ps.empty()) return;
    int set_index= get_index(ps[0]);
    for (unsigned int i=1; i< ps.size(); ++i) {
      int index= get_index(ps[i]);
      disjoint_sets_.union_set(set_index, index);
    }
    sets_.clear();
  }
  void DisjointSetsSubsetFilterTable::add_pair(const ParticlePair &pp) {
    int set_index= get_index(pp[0]);
    int index= get_index(pp[1]);
    disjoint_sets_.union_set(set_index, index);
    sets_.clear();
  }


IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Exclusion, {
    Ints states;
    for (unsigned int i=0; i< members.size(); ++i) {
      if (members[i] != -1) {
        states.push_back( state[members[i]] );
      }
    }
    std::sort(states.begin(), states.end());
    return std::unique(states.begin(), states.end())==states.end();
  });

IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Equality, {
    unsigned int base=0;
    while (base < members.size() && members[base]==-1) ++base;
    for (unsigned int i=base+1; i< members.size(); ++i) {
      if (members[i] != -1) {
        if (state[members[i]] != state[members[base]]) return false;
      }
    }
    return true;
  });


IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Equivalence, {
    //IMP_LOG(TERSE, "State is " << state << " and ");
    //logit(TERSE, members);
    //IMP_LOG(TERSE, " are the members." << std::endl);
    unsigned int last=0;
    while (last < members.size() && members[last]==-1) ++last;
    for (unsigned int i=last+1; i< members.size(); ++i) {
      if (members[i] != -1) {
        if (state[members[last]] > state[members[i]]) return false;
        last=i;
      }
    }
    //IMP_LOG(TERSE, "ok" << std::endl);
    return true;
  });


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
::ListSubsetFilterTable(ParticleStatesTable *pst):
  SubsetFilterTable("ListSubsetFilterTable%1%"), pst_(pst)
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
    states_.push_back(boost::dynamic_bitset<>(num));
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

IMPDOMINO_END_NAMESPACE
