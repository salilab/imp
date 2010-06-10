/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/domino2_config.h>
#include <map>
#include <set>
#include <boost/version.hpp>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/particle_states.h>
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

  class Permutation {
    boost::scoped_array<int> p_;
    unsigned int n_;
    unsigned int k_;
    bool ok() const {
      for (unsigned int i=0; i< k_; ++i) {
        for (unsigned int j=0; j<i; ++j) {
          if (p_[i] == p_[j]) return false;
        }
      }
      return true;
    }
  public:
    Permutation() {
      n_=0;
      k_=0;
    }
    Permutation(const Permutation &o) {
      n_=o.n_;
      k_=o.k_;
    }
    void operator=(const Permutation &o) {
      n_=o.n_;
      k_=o.k_;
    }
    void initialize(unsigned int N, unsigned int K);
    void advance();
    bool done() const;
    void reset();
    unsigned int get_number() const;
    unsigned int get_size() const;
    unsigned int get_value(unsigned int i) const;
  };
  void Permutation::reset() {
    for (unsigned int i=0; i< k_; ++i) {
      p_[i]= i;
    }
  }
  bool Permutation::done() const {
    return p_[0]==-1;
  }
  void Permutation::initialize(unsigned int N, unsigned int K) {
    p_.reset(new int[K]);
    n_=N;
    k_=K;
    reset();
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      unsigned int ct=0;
      while (!done()) {
        /*for (unsigned int i=0; i< k_; ++i) {
          std::cout << get_value(i) << " ";
          }*/
        //std::cout << std::endl;
        ++ct;
        advance();
      }
      reset();
      IMP_INTERNAL_CHECK(ct==get_number(),
                         "Actual and computed sizes don't match "
                         << ct << " vs " << get_number()
                         << " for " << n_ << " and " << k_);
    }
    IMP_USAGE_CHECK(K <= N, "Too few choices for a " << K
                    << " permutation: " << N);
  }
  void Permutation::advance() {
    do {
      int i;
      for (i=k_-1; i>=0; --i) {
        ++p_[i];
        if (p_[i]==static_cast<int>(n_)) {
          p_[i]=0;
        } else {
          break;
        }
      }
      if (i==-1) {
        p_[0]=-1;
        break;
      }
    } while (!ok());
  }
  unsigned int Permutation::get_number() const {
    unsigned int ret=n_;
    for (unsigned int i=1; i< k_; ++i) {
      ret*= n_-i;
    }
    return ret;
  }
  unsigned int Permutation::get_size() const {
    return k_;
  }
  unsigned int Permutation::get_value(unsigned int i) const {
    IMP_USAGE_CHECK(i < k_, "Out of range");
    return p_[i];
  }


  class  DefaultSubsetStates: public SubsetStates {
  public:
    struct Class {
      unsigned int n;
      Ints indexes;
      Permutation value;
    };
    mutable std::vector<Class> classes_;
    mutable unsigned int cur_index_;
    const unsigned int n_;
    Pointer<ParticleStatesTable> pst_;
    void reset_state() const;
    void advance_state() const;
    DefaultSubsetStates(UF &equivalencies,
                        const std::set<Particle*> &seen,
                        Subset *s,
                        ParticleStatesTable *table);
    IMP_SUBSET_STATES(DefaultSubsetStates);
  };

  DefaultSubsetStates::DefaultSubsetStates(UF &equivalencies,
                                           const std::set<Particle*>& seen,
                                           Subset *s,
                                           ParticleStatesTable *table):
    SubsetStates("DefaultSubsetStates on "+s->get_name()),
    n_(s->get_number_of_particles()), pst_(table) {
    IMP_CHECK_OBJECT(table);
    IMP_CHECK_OBJECT(s);
    std::map<Particle*, int> indexes;
    unsigned int sz=n_;
    for (unsigned int i=0; i< sz; ++i) {
      indexes[s->get_particle(i)]=i;
    }
    std::map<Particle*, int> sets;
    for (unsigned int i=0; i< sz; ++i) {
      Particle*t=s->get_particle(i);
      Particle*set;
      if (seen.find(t) == seen.end()) {
        set= t;
      } else {
        set= equivalencies.find_set(t);
      }
      if (set ==t) {
        classes_.push_back(Class());
        sets[t]=classes_.size()-1;
        ParticleStates *e;
        e=pst_->get_particle_states(t);
        classes_.back().n= e->get_number_of_states();
      }
    }
    for (unsigned int i=0; i< sz; ++i) {
      Particle*t=s->get_particle(i);
      Particle*set;
      if (seen.find(t) == seen.end()) {
        set= t;
      } else {
        set= equivalencies.find_set(t);
      }
      unsigned int thisclass= sets.find(set)->second;
      IMP_INTERNAL_CHECK(classes_.size() > thisclass,
                         "Can't deal with class " << thisclass
                         << " because it is larger than "
                         << classes_.size());
      classes_[thisclass].indexes.push_back(indexes[t]);
    }
    for (unsigned int i=0; i< classes_.size(); ++i) {
      classes_[i].value.initialize(classes_[i].n,
                                   classes_[i].indexes.size());
    }
    /*IMP_IF_LOG(TERSE) {
      for (unsigned int i=0; i< classes_.size(); ++i) {
        IMP_LOG(TERSE, "Class " << i << " is " << classes_[i].n
                << " states for " << classes_[i].indexes.size()
                << " particles for "
                << classes_[i].value.get_number());
      }
      }*/
    reset_state();
  }

  void DefaultSubsetStates::reset_state() const {
    cur_index_=0;
    for (unsigned int i=0; i< classes_.size(); ++i) {
      classes_[i].value.reset();
    }
  }
  void DefaultSubsetStates::advance_state() const {
    ++cur_index_;
    for (unsigned int i=0; i < classes_.size(); ++i) {
      classes_[i].value.advance();
      if (classes_[i].value.done()) {
        classes_[i].value.reset();
      } else {
        break;
      }
    }
  }

  unsigned int DefaultSubsetStates::get_number_of_states() const {
    unsigned int ret=1;
    for (unsigned int i=0; i< classes_.size(); ++i) {
      ret*= classes_[i].value.get_number();
    }
    return ret;
  }
  SubsetState DefaultSubsetStates::get_state(unsigned int i) const {
    if (cur_index_ > i) {
      reset_state();
    }
    while (cur_index_ < i) {
      advance_state();
    }
    SubsetState ret(n_);
    for (unsigned int i=0; i< classes_.size(); ++i) {
      for (unsigned int j=0; j< classes_[i].indexes.size(); ++j) {
        ret[classes_[i].indexes[j]] = classes_[i].value.get_value(j);
      }
    }
    return ret;
  }

  void DefaultSubsetStates::do_show(std::ostream &out) const{}

}


DefaultSubsetStatesTable::DefaultSubsetStatesTable(ParticleStatesTable *pst):
  pst_(pst), equivalencies_(Rank(rank_), Parent(parent_)){}

void DefaultSubsetStatesTable::add_equivalency(Particle *a, Particle* b) {
  IMP_USAGE_CHECK(pst_
                  ->get_particle_states(a)->get_number_of_states()
                  == pst_
                  ->get_particle_states(b)->get_number_of_states(),
            "The number of states for the two particles does not match");
  if (seen_.find(a) == seen_.end()) {
    equivalencies_.make_set(a);
    seen_.insert(a);
  }
  if (seen_.find(b) == seen_.end()) {
    equivalencies_.make_set(b);
    seen_.insert(b);
  }
  equivalencies_.union_set(a,b);
}

SubsetStates* DefaultSubsetStatesTable::get_subset_states(Subset*s) const {
  return new DefaultSubsetStates(equivalencies_, seen_, s,
                                 pst_);
}

void DefaultSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
