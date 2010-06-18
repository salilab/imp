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
    void setup_permutations(UF &equivalencies,
                            const std::set<Particle*>& seen,
                            const Subset &s,
                            ParticleStatesTable *table,
                            std::vector<Ints> &classes);
    void setup_scores(const Subset &s, SubsetEvaluatorTable *set,
                      SubsetEvaluators &ses);
    std::vector<SubsetState> states_;
    BranchAndBoundSubsetStates(UF &equivalencies,
                        const std::set<Particle*> &seen,
                        double max,
                        const Subset &s,
                        ParticleStatesTable *table,
                        SubsetEvaluatorTable *set);
  IMP_SUBSET_STATES(BranchAndBoundSubsetStates);
};

  void BranchAndBoundSubsetStates::setup_permutations(UF &equivalencies,
                                               const std::set<Particle*>& seen,
                                               const Subset &s,
                                               ParticleStatesTable *table,
                                               std::vector<Ints> &classes) {
    classes.resize(s.size());
    for (unsigned int i=0; i < classes.size(); ++i) {
      if (seen.find(s[i]) == seen.end()) continue;
      Particle *ri= equivalencies.find_set(s[i]);
      for (unsigned int j=i+1; j< classes.size(); ++j) {
        if (seen.find(s[j]) == seen.end()) continue;
        Particle *rj= equivalencies.find_set(s[j]);
        if (ri==rj) {
          classes[i].push_back(j);
        }
      }
    }
  }

  void BranchAndBoundSubsetStates::setup_scores(const Subset &s,
                                         SubsetEvaluatorTable *set,
                                         SubsetEvaluators &ses) {
    ses.resize(s.size());
    for (unsigned int i=0; i < ses.size(); ++i) {
      //std::cout << "Getting evaluator for " << i << std::endl;
      ParticlesTemp pt(s.begin()+i, s.end());
      Subset s(pt, true);
      SubsetEvaluator* se= set->get_subset_evaluator(s);
      se->set_was_used(true);
      if (se) {
        ses[i]=se;
      }
    }
  }


BranchAndBoundSubsetStates::BranchAndBoundSubsetStates(UF &equivalencies,
                                         const std::set<Particle*>& seen,
                                         double max,
                                         const Subset &s,
                                         ParticleStatesTable *table,
                                         SubsetEvaluatorTable *set):
  SubsetStates("BranchAndBoundSubsetStates on "+s.get_name()) {
  IMP_OBJECT_LOG;
  IMP_LOG(VERBOSE, "Computing states for " << s.get_name()
          << " " << (set?"with":"without") << " filtering" << std::endl);
  std::vector<Ints> permutations;
  setup_permutations(equivalencies, seen, s, table, permutations);
  SubsetEvaluators evaluators;
  if (set) {
    setup_scores(s, set, evaluators);
  }
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
    for (unsigned int j=0; j < permutations[i].size(); ++j) {
      if (cur[permutations[i][j]]== cur[j]) {
        goto bad;
      }
    }
    if (set) {
      SubsetState ss(cur.begin()+i, cur.end());
      double score= evaluators[i]->get_score(ss);
      //std::cout << "score " << i << " is " << score << std::endl;
      if (score > max) {
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
  //std::cout << "Incrementing " << cur << " on " << current_digit << std::endl;
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

bool BranchAndBoundSubsetStates::get_is_state(const SubsetState &state) const {
  return std::binary_search(states_.begin(), states_.end(), state);
}

void BranchAndBoundSubsetStates::do_show(std::ostream &out) const{}

}


BranchAndBoundSubsetStatesTable
::BranchAndBoundSubsetStatesTable(ParticleStatesTable *pst):
  pst_(pst){}


SubsetStates* BranchAndBoundSubsetStatesTable
::get_subset_states(const Subset&s) const {
  typedef std::map<Particle*, Particle*> IParent;
  typedef std::map<Particle*, int> IRank;
  typedef boost::associative_property_map<IParent> Parent;
  typedef boost::associative_property_map<IRank > Rank;
  typedef boost::disjoint_sets<Rank, Parent> UF;
  IParent parent;
  IRank rank;
  Rank rrank(rank);
  Parent rparent(parent);
  UF equivalencies(rrank, rparent);
  // for some reason boost disjoint sets doesn't provide a way to see
  // if an item is a set
  std::set<Particle*> seen;
  for (unsigned int i=0; i< s.size(); ++i) {
    Particle *a= s[i];
    for (unsigned int j=0; j< i; ++j) {
      Particle *b= s[j];
      if (pst_->get_particle_states(a)
          == pst_->get_particle_states(b)) {
        if (seen.find(a) == seen.end()) {
          equivalencies.make_set(a);
          seen.insert(a);
        }
        if (seen.find(b) == seen.end()) {
          equivalencies.make_set(b);
          seen.insert(b);
        }
        equivalencies.union_set(a,b);
      }
    }
  }
  double max;
  if (get_has_sampler()) {
    max= get_sampler()->get_maximum_score();
  } else {
    max= std::numeric_limits<double>::max();
  }
  return new BranchAndBoundSubsetStates(equivalencies, seen, max,
                                 s,
                                 pst_, set_);
}

void BranchAndBoundSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
