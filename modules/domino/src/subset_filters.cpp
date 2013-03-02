/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/particle_states.h>
#include <IMP/base/log_macros.h>
#include <IMP/core/XYZ.h>
#include <IMP/domino/internal/inference_utility.h>
#include <IMP/random.h>
#include <limits>


IMPDOMINO_BEGIN_NAMESPACE
SubsetFilter::SubsetFilter(std::string name): Object(name){}
SubsetFilter::~SubsetFilter(){}
SubsetFilterTable::~SubsetFilterTable(){}



/***************************MINIMUM ******************************/
namespace {
/** A minimum restraint score based SubsetFilter.
    See MinimumRestraintScoreSubsetFilterTable.
 */
class IMPDOMINOEXPORT MinimumRestraintScoreSubsetFilter: public SubsetFilter {
  Pointer<RestraintCache> rc_;
  RestraintsTemp rs_;
  Slices slices_;
  unsigned int max_;
  friend class IMP::domino::MinimumRestraintScoreSubsetFilterTable;
  MinimumRestraintScoreSubsetFilter(RestraintCache *rc,
                                     Subset s,
                                    const RestraintsTemp &rs,
                                    int max):
    SubsetFilter("Minimum restraint score filter"),
      rc_(rc), rs_(rs),
      max_(max){
        for (unsigned int i=0; i< rs_.size(); ++i) {
          slices_.push_back(rc->get_slice(rs_[i],s));
        }
  }
public:
  IMP_SUBSET_FILTER(MinimumRestraintScoreSubsetFilter);
};

bool MinimumRestraintScoreSubsetFilter
::get_is_ok(const Assignment &state) const{
  IMP_OBJECT_LOG;
  set_was_used(true);
  unsigned int bad_count=0;
  for (unsigned int i=0; i< rs_.size(); ++i) {
    double s= rc_->get_score(rs_[i], slices_[i].get_sliced(state));
    if (s >= std::numeric_limits<double>::max()) {
      ++bad_count;
      if (bad_count>max_) break;
    }
  }
  return bad_count <= max_;
}

void MinimumRestraintScoreSubsetFilter::do_show(std::ostream &) const{
}
}




MinimumRestraintScoreSubsetFilterTable
::MinimumRestraintScoreSubsetFilterTable(const RestraintsTemp &rs,
                                         RestraintCache *rc,
                                         int max):
  SubsetFilterTable("MinimumRestraintScoreSubsetFilterTable%1%"),
  rc_(rc), rs_(rs.begin(), rs.end()), max_violated_(max){
  IMP_USAGE_CHECK(rc_, "Must pass a restriant cache");
  std::sort(rs_.begin(), rs_.end());
  }

RestraintsTemp MinimumRestraintScoreSubsetFilterTable
::get_restraints(const Subset &s,
                 const Subsets &excluded) const {
  RestraintsTemp all= rc_->get_restraints(s, excluded);
  // if there are no new ones, return nothing
  if (all.empty()) return RestraintsTemp();
  // otherwise, we want them all, not just new
  all= rc_->get_restraints(s, Subsets());
  RestraintsTemp ret;
  for (unsigned int i=0; i< all.size(); ++i) {
    if (std::binary_search(rs_.begin(), rs_.end(), all[i])) {
      ret.push_back(all[i]);
    }
  }
  return ret;
}

SubsetFilter*
MinimumRestraintScoreSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &excluded) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  RestraintsTemp rs= get_restraints(s, excluded);
  if (rs.empty()) return nullptr;
  else {
    return new MinimumRestraintScoreSubsetFilter(rc_, s,
                                                 rs,
                                                 max_violated_);
  }
}

double MinimumRestraintScoreSubsetFilterTable
::get_strength(const Subset &s,
               const Subsets &excluded) const {
  set_was_used(true);
  unsigned int nr= get_restraints(s, excluded).size();
  return 1-std::pow(.5,
                    static_cast<int>(nr));
}


void MinimumRestraintScoreSubsetFilterTable::do_show(std::ostream &) const {
}

// ******************************* Disjoint sets ********************

namespace {


  double get_default_strength(const IMP::domino::Subset &s,
                              const IMP::domino::Subsets &,
                              const Ints &members) {
    unsigned int sz=0;
    for (unsigned int i=0; i < members.size(); ++i) {
      if (members[i] >=0) ++sz;
    }
    return std::pow(.1, static_cast<double>(s.size()-sz));
  }


  template <class Filter, class Next>
  class  DisjointSetsSubsetFilter: public SubsetFilter {
    base::Vector<Ints> sets_;
  public:
    DisjointSetsSubsetFilter(const base::Vector<Ints> &sets):
      SubsetFilter("DisjointSetsSubsetFilter%1%"),
      sets_(sets) {
      IMP_LOG_TERSE( "Created disjoint set subset filter with ");
      IMP_IF_LOG(TERSE) {
        for (unsigned int i=0; i < sets.size(); ++i) {
          IMP_LOG_TERSE( sets_[i]);
        }
        IMP_LOG_TERSE( std::endl);
      }
    }
    IMP_OBJECT(DisjointSetsSubsetFilter);
    bool get_is_ok(const Assignment &state) const{
      IMP_OBJECT_LOG;
      set_was_used(true);
      Filter f;
      for (unsigned int i=0; i< sets_.size(); ++i) {
        if (!f(state, sets_[i])) return false;
      }
      return true;
    }
    int get_next_state(int pos, const Assignment& state) const {
      for (unsigned int i=0; i< sets_.size(); ++i) {
        for (unsigned int j=0; j< sets_[i].size(); ++j) {
          if (sets_[i][j]==pos) {
            return Next()(pos, state, sets_[i]);
          }
        }
      }
      IMP_FAILURE("No knowledge of current pos");
      return SubsetFilter::get_next_state(pos, state);
    }
  };
  template <class Filter, class Next>
  void  DisjointSetsSubsetFilter<Filter, Next>::do_show(std::ostream &)
    const{}


  template <class FF, class Next>
  DisjointSetsSubsetFilter<FF, Next>*
  get_disjoint_set_filter(std::string name,
                          const Subset &s,
                          base::LogLevel ll,
                          const base::Vector<Ints> &all,
                          const Ints &) {
    if (all.empty()) return nullptr;
    typedef DisjointSetsSubsetFilter<FF, Next> CF;
    IMP_NEW(CF, f, (all));
    f->set_log_level(ll);
    std::ostringstream oss;
    oss << name << " for " << s;
    f->set_name(oss.str());
    return f.release();
  }


  template <class SF>
  double get_disjoint_set_strength(const IMP::domino::Subset &s,
                                   const IMP::domino::Subsets &excluded,
                        const base::Vector<Ints> &all,
                                   const Ints &){
    double r=1;
    SF str;
    for (unsigned int i=0; i< all.size(); ++i) {
      double rc=str(s, excluded, all[i]);
      r*=(1-rc);
    }
    return 1-r;
  }


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
    IMP::base::map<ParticleStates*, int> map;
    ParticlesTemp allps= pst_->get_particles();
    base::Vector<ParticlesTemp> allsets;
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

  base::Vector<ParticlesTemp> all(elements_.size());
  for (unsigned int i=0; i< elements_.size(); ++i) {
    int set= disjoint_sets_.find_set(i);
    all[set].push_back(elements_[i]);
  }
  for (unsigned int i=0; i< all.size(); ++i) {
    if (all[i].size() >1 ) {
      sets_.push_back(all[i]);
      std::sort(sets_.back().begin(), sets_.back().end());
    }
  }
  for (unsigned int i=0; i < sets_.size(); ++i) {
    for (unsigned int j=0; j< sets_[i].size(); ++j) {
      set_indexes_[sets_[i][j]]=j;
    }
  }
  IMP_IF_LOG(TERSE) {
    IMP_LOG_TERSE( "Sets are:\n");
    for (unsigned int i=0; i< sets_.size(); ++i) {
      IMP_LOG_TERSE( sets_[i]);
      IMP_LOG_TERSE( std::endl);
    }
  }
}

void
DisjointSetsSubsetFilterTable::get_indexes(const Subset &s,
                                           const Subsets &excluded,
                             base::Vector<Ints> &ret,
                                           int lb,
                                           Ints &used) const {
  for (unsigned int i=0; i< get_number_of_sets(); ++i) {
    Ints index= IMP::domino::get_partial_index(get_set(i),
                                               s, excluded);
    /*std::cout << "Index of " << s << " wrt " << Particles(get_set(i))
      << " is " << internal::AsIndexes(index) << std::endl;*/
    int ct =0;
    for (unsigned int j=0; j< index.size(); ++j) {
      if (index[j] != -1) {
        ++ct;
      }
    }
    if (ct >lb) {
      ret.push_back(index);
      used.push_back(i);
    }
  }
}


void DisjointSetsSubsetFilterTable::add_set(const ParticlesTemp &ps) {
  IMP_USAGE_CHECK(!pst_, "Defining sets through the ParticleStatesTable"
                  << " and explicitly are mutually exclusive.");
  if (ps.empty()) return;
  int set_index= get_index(ps[0]);
  for (unsigned int i=1; i< ps.size(); ++i) {
    int index= get_index(ps[i]);
    disjoint_sets_.union_set(set_index, index);
  }
  sets_.clear();
}
void DisjointSetsSubsetFilterTable::add_pair(const ParticlePair &pp) {
  IMP_USAGE_CHECK(!pst_, "Defining sets through the ParticleStatesTable"
                  << " and explicitly are mutually exclusive.");
  int set_index= get_index(pp[0]);
  int index= get_index(pp[1]);
  disjoint_sets_.union_set(set_index, index);
  sets_.clear();
}

namespace {
  int get_next_exclusion(int pos, const Assignment& state,
                         const Ints &set) {
    Ints used;
    for (unsigned int i=0; i<set.size();++i){
      if (set[i]>=0) {
        used.push_back(state[set[i]]);
      }
    }
    std::sort(used.begin(), used.end());
    int st= state[pos]+1;
    Ints::const_iterator it=std::lower_bound(used.begin(), used.end(), st);
    while (it != used.end() && *it==st) {
      ++st;
      ++it;
    }
    IMP_USAGE_CHECK(!std::binary_search(used.begin(), used.end(), st),
                    "Found");
    IMP_INTERNAL_CHECK(st > state[pos], "Too low an index returned.");
    return st;
  }
  int get_next_equality(int pos, const Assignment& state,
                         const Ints &set) {
    for (unsigned int i=0; i<set.size();++i){
      if (set[i] != -1 && state[set[i]] != state[pos]) {
        if (state[set[i]] > state[pos]) {
          return state[set[i]];
        } else {
          return std::numeric_limits<int>::max();
        }
      }
    }
    IMP_THROW("!found", ValueException);
    return -1;
  }
}


IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Exclusion, {
    for (unsigned int i=0; i< members.size(); ++i) {
      if (members[i] != -1) {
        int si= state[members[i]];
        for (unsigned int j=0; j < i; ++j) {
          if (members[j] != -1) {
            if (si== state[members[j]]) return false;
          }
        }
      }
    }
    return true;
  },return get_default_strength(s, excluded, members),
  return get_next_exclusion(pos, state, set));

IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Equality, {
    unsigned int base=0;
    while (base < members.size() && members[base]==-1) ++base;
    for (unsigned int i=base+1; i< members.size(); ++i) {
      if (members[i] != -1) {
        if (state[members[i]] != state[members[base]]) return false;
      }
    }
    return true;
  }, return get_default_strength(s, excluded, members),
  return get_next_equality(pos, state, set));




namespace {
  double get_sorted_strength(const IMP::domino::Subset &s,
                             const IMP::domino::Subsets &,
                             const Ints &members) {
    /*IMP_LOG(SILENT, "For " << s << " ");
      logit(SILENT, members);*/
    int count=0;
    bool gap=false;
    for (unsigned int i=0; i< members.size(); ++i) {
      if (members[i] != -1){
        if (gap || static_cast<unsigned int>(members[i]) < i) {
          /*IMP_LOG(SILENT, " not packed " << i
            << " " << count << std::endl);*/
          return 0;
        }
        ++count;
      } else {
        gap=true;
      }
    }
    //IMP_LOG(SILENT, " returning for set " << count << std::endl);
    double ret= std::pow(.1, static_cast<double>(s.size()-count));
    return ret;
  }

  int get_next_permutation(int pos, const Assignment& state,
                           const Ints &set) {
    int mx=-1;
    for (unsigned int i=0; i<set.size();++i){
      if (set[i]>=0){
        mx=std::max(mx, state[set[i]]);
      }
    }
    int ret= std::max(mx, state[pos]+1);
    IMP_INTERNAL_CHECK(ret > state[pos],
                       "Too low a permutation index returned: "
                       << ret << " vs " << state[pos]);
    return ret;
  }
}

IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(Equivalence, {
    IMP_LOG_TERSE( "State is " << state << " and ");
    IMP_LOG_TERSE( members);
    IMP_LOG_TERSE( " are the members." << std::endl);
    int last=-1;
    for (unsigned int i=0; i< members.size(); ++i) {
      if (members[i]==-1) continue;
      // it is too low an index to work globally
      /*if (state[members[i]] < members[i]) {
        IMP_LOG_VERBOSE( "Rejected due to index being too low"
                << state << " at " << members[i]
                << std::endl);
        return false;
        }*/
      if (last > state[members[i]]) {
        IMP_LOG_VERBOSE( "Rejected due order"
                << state << " at " << i << " that is "
                << state[members[i]]
                << " vs " << last << std::endl);
        return false;
      }
      last= state[members[i]];
    }
    //IMP_LOG_TERSE( "ok" << std::endl);
    return true;
  }, return get_sorted_strength(s, excluded, members),
  return get_next_permutation(pos, state, set));


namespace {

  int get_next_equivalence_exclusion(int pos, const Assignment& state,
                                    const Ints &set) {
    int max=0;
    for (unsigned int i=0; i<set.size();++i){
      max= std::max(max, state[set[i]]+1);
      if (set[i]==pos){
        // could have failed because too low for space below, or
        // too high for space after
        // or already covered
        return std::max<unsigned int>(i, max);
      }
    }
    IMP_THROW("!found", ValueException);
    return -1;
  }
}

IMP_DISJOINT_SUBSET_FILTER_TABLE_DEF(EquivalenceAndExclusion, {
    int last=-1;
    for (unsigned int i=0; i< members.size(); ++i) {
      if (members[i] != -1) {
        unsigned int si= state[members[i]];
        if (si < i || static_cast<int>(si) <= last) return false;
        last=state[members[i]];
      }
    }
    return true;
  },return get_sorted_strength(s, excluded, members),
  return get_next_equivalence_exclusion(pos, state, set));


// **************************************** List ********************


namespace {
  class  ListSubsetFilter: public SubsetFilter {
    Pointer<const ListSubsetFilterTable> keepalive_;
    Ints indexes_;
  public:
    ListSubsetFilter(const ListSubsetFilterTable *ka,
                     const Ints indexes):
      SubsetFilter("List score filter"),
      keepalive_(ka), indexes_(indexes) {
    }
    int get_next_state(int pos,
                       const Assignment& state) const;
    IMP_SUBSET_FILTER(ListSubsetFilter);
  };

  bool ListSubsetFilter::get_is_ok(const Assignment &state) const{
    set_was_used(true);
    ++keepalive_->num_test_;
    for (unsigned int i=0; i < state.size(); ++i) {
      if (indexes_[i]>=0) {
        if (!keepalive_->states_[indexes_[i]].test(state[i])) {
          IMP_LOG_VERBOSE( "Rejecting state " << state
                  << " due to particle " << state[i] << std::endl);
          return false;
        }
      }
    }
    ++keepalive_->num_ok_;
    return true;
  }

  int ListSubsetFilter::get_next_state(int pos,
                                       const Assignment& state) const {
    int ret= keepalive_->states_[indexes_[pos]].find_next(state[pos]);
    if (ret==-1) return keepalive_->states_[indexes_[pos]].size();
    return ret;
  }

  void ListSubsetFilter::do_show(std::ostream &) const{}
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
    return -1;
  } else {
    return map_.find(p)->second;
  }
}

void ListSubsetFilterTable
::load_indexes(const Subset &s,
               Ints &indexes) const {
  ParticlesTemp cur(s.begin(), s.end());
  indexes.resize(cur.size(), -1);
  for (unsigned int i=0; i< cur.size(); ++i) {
    indexes[i]= get_index(cur[i]);
  }
}

SubsetFilter*
ListSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &) const {
  set_was_used(true);
  Ints indexes;
  load_indexes(s, indexes);
  IMP_NEW(ListSubsetFilter, ret, (this, indexes));
  ret->set_log_level(get_log_level());
  return ret.release();
}

double ListSubsetFilterTable::get_strength(const Subset &s,
                                           const Subsets &) const {
  // really bad estimate
  set_was_used(true);
  Ints indexes;
  load_indexes(s, indexes);
  int sz=0;
  for (unsigned int i=0; i< s.size(); ++i) {
    if (indexes[i]>=0) ++sz;
  }
  return 1-std::pow(.5, static_cast<int>(sz));
}



void ListSubsetFilterTable::do_show(std::ostream &) const {
}

void ListSubsetFilterTable::set_allowed_states(Particle *p,
                                               const Ints &states) {
  int index;
  if (map_.find(p) != map_.end()) {
    index= map_.find(p)->second;
  } else {
    index= states_.size();
    states_.push_back(boost::dynamic_bitset<>());
    map_[p]=index;
  }
  boost::dynamic_bitset<> s(pst_->get_particle_states(p)
                            ->get_number_of_particle_states(),
                            false);
  for (unsigned int i=0; i< states.size(); ++i) {
    s[states[i]]=true;
  }
  states_[index]=s;
}


void ListSubsetFilterTable::mask_allowed_states(Particle *p,
                     const boost::dynamic_bitset<> &bs) {
  if (map_.find(p) == map_.end()) {
    map_[p]=states_.size();
    states_.push_back(bs);
  } else {
    int s= map_.find(p)->second;
    states_[s]&=bs;
  }
}






/*************************************************************************/



namespace {
  struct CP {
    bool operator()(const IntPair &pa,
                    const IntPair &pb) const {
      if (pa.first < pb.first) return true;
      else if (pa.first > pb.first) return false;
      else if (pa.second < pb.second) return true;
      else return false;
    }
  };

  class  PairListSubsetFilter: public SubsetFilter {
    IntPairs indexes_;
    base::Vector<IntPairs> allowed_;
  public:
    PairListSubsetFilter(const IntPairs &i,
                         const base::Vector<IntPairs> &a):
      SubsetFilter("Pair list score filter"),
      indexes_(i), allowed_(a) {
    }
    IMP_SUBSET_FILTER(PairListSubsetFilter);
  };

  bool PairListSubsetFilter::get_is_ok(const Assignment &state) const{
    for (unsigned int i=0; i< indexes_.size(); ++i) {
      IntPair ip(state[indexes_[i].first], state[indexes_[i].second]);
      bool c= std::binary_search(allowed_[i].begin(),
                                 allowed_[i].end(), ip, CP());
      if (!c) return false;
    }
    return true;
  }

  void PairListSubsetFilter::do_show(std::ostream &) const{}
}


void PairListSubsetFilterTable
::fill(const Subset &s,
            const Subsets &e,
            IntPairs& indexes,
       base::Vector<IntPairs>& allowed) const {
for (unsigned int i=0; i< s.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      ParticlePair pp(s[j], s[i]);
      if (allowed_.find(pp) == allowed_.end()) continue;
      bool fp=false;
      for (unsigned int k=0; k< e.size(); ++k) {
        bool f0=false, f1=false;
        for (unsigned int l=0; l < e[k].size(); ++l) {
          if (e[k][l]==pp[0]) {
            f0=true;
          } else if (e[k][l]==pp[1]) {
            f1=true;
          }
          if (f0&&f1) {
            fp=true;
            break;
          }
        }
        if (fp) break;
      }
      if (fp) continue;
      indexes.push_back(IntPair(j,i));
      allowed.push_back(allowed_.find(pp)->second);
    }
  }
}

SubsetFilter*
PairListSubsetFilterTable
::get_subset_filter(const Subset &s,
                    const Subsets &e) const {
  set_was_used(true);
  IntPairs indexes;
  base::Vector<IntPairs> allowed;
  fill(s,e,indexes, allowed);
  if (!indexes.empty()) {
    return new PairListSubsetFilter(indexes, allowed);
  } else {
    return nullptr;
  }
}

double PairListSubsetFilterTable::get_strength(const Subset &s,
                                           const Subsets &e) const {
  IntPairs indexes;
  base::Vector<IntPairs> allowed;
  fill(s,e,indexes, allowed);
  return 1-std::pow(.9, static_cast<double>(indexes.size()));
}

PairListSubsetFilterTable::PairListSubsetFilterTable(){}

void PairListSubsetFilterTable::do_show(std::ostream &) const {
}

void PairListSubsetFilterTable::set_allowed_states(ParticlePair p,
                                                   const IntPairs &states) {
  IMP_USAGE_CHECK(allowed_.find(p) == allowed_.end(),
                  "Allowed states for " << p
                  << " already set.");
  if (p[0] < p[1]) p= ParticlePair(p[1], p[0]);
  allowed_[p]=states;
  std::sort(allowed_[p].begin(), allowed_[p].end(), CP());
}



/*************************************************************************/



namespace {
  class  ProbabilisticSubsetFilter: public SubsetFilter {
    double p_;
    mutable boost::uniform_real<> r_;
  public:
    ProbabilisticSubsetFilter(double p):
      SubsetFilter("ProbabilisticSubsetFilter %1%"),
      p_(p), r_(0,1) {
    }
    IMP_SUBSET_FILTER(ProbabilisticSubsetFilter);
  };

  bool ProbabilisticSubsetFilter::get_is_ok(const Assignment &) const{
    return r_(random_number_generator) <p_;
  }

  void ProbabilisticSubsetFilter::do_show(std::ostream &) const{}
}


SubsetFilter*
ProbabilisticSubsetFilterTable
::get_subset_filter(const Subset &,
                    const Subsets &e) const {
  set_was_used(true);
  if (e.size() >1 && leaves_only_) return 0;
  else{
    IMP_NEW(ProbabilisticSubsetFilter, ret, (p_));
    ret->set_log_level(get_log_level());
    return ret;
  }
}

double ProbabilisticSubsetFilterTable::get_strength(const Subset &,
                                           const Subsets &e) const {
  if (e.size() >0 && leaves_only_) return 0;
  else return p_;
}

ProbabilisticSubsetFilterTable
::ProbabilisticSubsetFilterTable(double p,
                                 bool lo):
  SubsetFilterTable("ProbabilisticSubsetFilterTable %1%"),
  p_(p), leaves_only_(lo){}

void ProbabilisticSubsetFilterTable::do_show(std::ostream &) const {
}





// filtetring==========================================

namespace {
class RestraintScoreSubsetFilter: public SubsetFilter {
  OwnerPointer<RestraintCache> cache_;
  RestraintsTemp rs_;
  Slices slices_;
public:
  RestraintScoreSubsetFilter(RestraintCache *cache,
                             const RestraintsTemp rs,
                             const Subset&s,
                             const Subsets &):
      SubsetFilter("RestraintScoreSubsetFilter%1%"),
      cache_(cache),
      rs_(rs) {
    for (unsigned int i=0; i < rs_.size(); ++i) {
      slices_.push_back(cache->get_slice(rs_[i], s));
    }
  }
  IMP_SUBSET_FILTER(RestraintScoreSubsetFilter);
};


bool RestraintScoreSubsetFilter
::get_is_ok(const Assignment& state) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  for (unsigned int i=0; i< rs_.size(); ++i) {
    Assignment substate=slices_[i].get_sliced(state);
    double score=cache_->get_score(rs_[i], substate);
    IMP_LOG_VERBOSE( "Score for restraint " << Showable(rs_[i])
            << " with assignment " << substate << " is " << score
            << std::endl);
    if (score >= std::numeric_limits<double>::max()) return false;
  }
  return true;
}

void RestraintScoreSubsetFilter
::do_show(std::ostream &out) const {
  out << "restraints: " << Showable(rs_) << std::endl;
  out << "slices: " << Showable(slices_) << std::endl;
}

}

RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(RestraintCache *cache):
  SubsetFilterTable("RestraintScoreSubsetFilterTable%1%"),
  cache_(cache){}



RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(RestraintSet *rs,
                                ParticleStatesTable *pst):
  SubsetFilterTable("RestraintScoreSubsetFilterTable%1%"),
  cache_(new RestraintCache(pst)), rs_(1, rs) {
}
RestraintScoreSubsetFilterTable
::RestraintScoreSubsetFilterTable(RestraintsTemp rs,
                                  ParticleStatesTable *pst):
  SubsetFilterTable("RestraintScoreSubsetFilterTable%1%"),
  cache_(new RestraintCache(pst)), rs_(rs.begin(), rs.end()){
}


SubsetFilter*
RestraintScoreSubsetFilterTable
::get_subset_filter(const Subset&s,
                    const Subsets &excluded) const {
  if (!rs_.empty()) {
    cache_->add_restraints(get_as<RestraintsTemp>(rs_));
    rs_.clear();
  }
  RestraintsTemp rs= cache_->get_restraints(s, excluded);
  if (rs.empty()) {
    IMP_LOG_TERSE( "No restraints on subset " << s
            << " with excluded " << excluded << std::endl);
    return nullptr;
  } else {
    return new RestraintScoreSubsetFilter(cache_, rs, s, excluded);
  }
}
double RestraintScoreSubsetFilterTable
::get_strength(const Subset&s,
               const Subsets &excluded) const {
  int n= cache_->get_restraints(s, excluded).size();
  return 1.0-1.0/(n+1.0);
}

void RestraintScoreSubsetFilterTable
::do_show(std::ostream &out) const {
  out << "cache: " << Showable(cache_) << std::endl;
}

IMPDOMINO_END_NAMESPACE
