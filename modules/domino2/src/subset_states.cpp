/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/domino2_config.h>
#include <map>

// According to boost docs, this should not be needed, but it appears to be
IMP_BEGIN_NAMESPACE
class Particle;
IMP_END_NAMESPACE
namespace boost {
  IMP::Particle* get(const std::map<IMP::Particle*, IMP::Particle*>&map,
                     IMP::Particle *p) {
    return map.find(p)->second;
  }
  void put( std::map<IMP::Particle*, IMP::Particle*>&map,
            IMP::Particle *p, IMP::Particle *v) {
    map[p]=v;
  }
  int get(const std::map<IMP::Particle*, int>&map,
                     IMP::Particle *p) {
    return map.find(p)->second;
  }
  void put( std::map<IMP::Particle*, int>&map,
            IMP::Particle *p, int v) {
    map[p]=v;
  }
}

#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/particle_states.h>
#include <IMP/core/XYZ.h>



IMPDOMINO2_BEGIN_NAMESPACE
SubsetStates::~SubsetStates(){}
SubsetStatesTable::~SubsetStatesTable(){}



namespace {

  typedef std::map<Particle*, Particle*> Parent;
  typedef std::map<Particle*, int> Rank;
  typedef boost::disjoint_sets<Rank, Parent> UF;

  class Permutation {
    boost::scoped_array<unsigned int> p_;
    unsigned int n_;
    unsigned int k_;
    void increment();
    bool ok() const;
    void reset();
    bool final() const;
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
    bool advance();
    unsigned int get_number() const;
    unsigned int get_size() const;
    unsigned int get_value(unsigned int i) const;
  };


  void Permutation::increment() {
    for (unsigned int i=0; i< k_; ++i) {
      ++p_[i];
      if (p_[i]==n_) {
        p_[i]=0;
      } else {
        break;
      }
    }
  }
  bool Permutation::ok() const {
    for (unsigned int i=0; i< k_; ++i) {
      for (unsigned int j=0; j<i; ++j) {
        if (p_[i] == p_[j]) return false;
      }
    }
    return true;
  }
  void Permutation::reset() {
    for (unsigned int i=0; i< k_; ++i) {
      p_[i]= i;
    }
  }
  bool Permutation::final() const {
    for (unsigned int i=0; i< k_; ++i) {
      if (p_[i] != n_-k_+i) return false;
    }
    return true;
  }
  void Permutation::initialize(unsigned int N, unsigned int K) {
    p_.reset(new unsigned int[K]);
    n_=N;
    k_=K;
    reset();
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      unsigned int ct=0;
      while (!advance()) ++ct;
      reset();
      IMP_INTERNAL_CHECK(ct==get_number(),
                         "Actual and computed sizes don't match "
                         << ct << " vs " << get_number());
    }
    IMP_USAGE_CHECK(K <= N, "Too few choices for a " << K
                    << " permutation: " << N);
  }
  bool Permutation::advance() {
    if (final()) {
      reset();
      return true;
    }
    do {
      increment();
    } while (!ok());
    return false;
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
    mutable Ints cur_state_;

    void scatter(unsigned int class_index) const;
    void reset_state() const;
    void advance_state() const;
    DefaultSubsetStates(UF &equivalencies,
                        Subset *s,
                        ParticleStatesTable *table);
    IMP_SUBSET_STATES(DefaultSubsetStates);
  };

  DefaultSubsetStates::DefaultSubsetStates(UF &equivalencies,
                                           Subset *s,
                                           ParticleStatesTable *table):
    SubsetStates(s, table) {
    std::map<Particle*, int> indexes;
    unsigned int sz=s->get_number_of_particles();
    for (unsigned int i=0; i< sz; ++i) {
      indexes[s->get_particle(i)]=i;
    }
    std::map<Particle*, int> sets;
    for (unsigned int i=0; i< sz; ++i) {
      Particle*t=s->get_particle(i);
      Particle*set= equivalencies.find_set(t);
      if (set ==t) {
        classes_.push_back(Class());
        sets[t]=classes_.size()-1;
        ParticleStates *e;
        e=get_particle_states_table()->get_particle_states(t);
        classes_.back().n= e->get_number_of_states();
      }
    }
    for (unsigned int i=0; i< sz; ++i) {
      Particle*t=s->get_particle(i);
      Particle* set= equivalencies.find_set(t);
      unsigned int thisclass= sets[set];
      classes_[thisclass].indexes.push_back(indexes[t]);
    }
    reset_state();
  }


  void DefaultSubsetStates::scatter(unsigned int class_index) const {
    for (unsigned int j=0; j< classes_[class_index].indexes.size(); ++j) {
      cur_state_[classes_[class_index].indexes[j]]
        = classes_[class_index].value.get_value(j);
    }
  }
  void DefaultSubsetStates::reset_state() const {
    cur_state_.clear();
    cur_state_.resize(get_subset()->get_number_of_particles(), -1);
    for (unsigned int i=0; i< classes_.size(); ++i) {
      classes_[i].value.initialize(classes_[i].n, classes_[i].indexes.size());
      scatter(i);
    }
  }
  void DefaultSubsetStates::advance_state() const {
    ++cur_index_;
    for (unsigned int i=0; i < classes_.size(); ++i) {
      bool wrap= classes_[i].value.advance();
      if (!wrap) break;
    }
  }

  unsigned int DefaultSubsetStates::get_number_of_states() const {
    unsigned int ret=1;
    for (unsigned int i=0; i< classes_.size(); ++i) {
      ret*= classes_[i].value.get_number();
    }
    return ret;
  }
  Ints DefaultSubsetStates::get_state(unsigned int i) const {
    if (cur_index_ > i) {
      reset_state();
    }
    while (cur_index_ < i) {
      advance_state();
    }
    return cur_state_;
  }

  void DefaultSubsetStates::do_show(std::ostream &out) const{}

}


DefaultSubsetStatesTable::DefaultSubsetStatesTable():
  equivalencies_(Rank(), Parent()){}

void DefaultSubsetStatesTable::add_equivalency(Particle *a, Particle* b) {
  IMP_USAGE_CHECK(get_particle_states_table()
                  ->get_particle_states(a)->get_number_of_states()
                  == get_particle_states_table()
                  ->get_particle_states(b)->get_number_of_states(),
                  "The number of states for the two particles does not match");
  equivalencies_.union_set(a,b);
}

SubsetStates* DefaultSubsetStatesTable::get_subset_states(Subset*s) const {
  return new DefaultSubsetStates(equivalencies_, s,
                                 get_particle_states_table());
}

void DefaultSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
