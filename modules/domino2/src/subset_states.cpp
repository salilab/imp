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
    void setup_classes(UF &equivalencies,
                       const std::set<Particle*>& seen,
                       Subset *s,
                       ParticleStatesTable *table,
                       std::vector<Class> &classes);
    void reset_state() const;
    void advance_state() const;
    std::vector<SubsetState> states_;
    DefaultSubsetStates(UF &equivalencies,
                        const std::set<Particle*> &seen,
                        Subset *s,
                        ParticleStatesTable *table,
                        SubsetEvaluatorTable *set);
    IMP_SUBSET_STATES(DefaultSubsetStates);
  };

  DefaultSubsetStates::DefaultSubsetStates(UF &equivalencies,
                                           const std::set<Particle*>& seen,
                                           Subset *s,
                                           ParticleStatesTable *table,
                                           SubsetEvaluatorTable *set):
    SubsetStates("DefaultSubsetStates on "+s->get_name()) {
    std::vector<Class> classes;
    IMP_CHECK_OBJECT(table);
    IMP_CHECK_OBJECT(s);
    setup_classes(equivalencies, seen, s, table, classes);

    for (unsigned int i=0; i< classes.size(); ++i) {
      classes[i].value.reset();
    }
    const unsigned int sz=s->get_number_of_particles();
    SubsetState ret(sz);
    while (true) {
      {
        for (unsigned int i=0; i< classes.size(); ++i) {
          for (unsigned int j=0; j< classes[i].indexes.size(); ++j) {
            ret[classes[i].indexes[j]] = classes[i].value.get_value(j);
          }
        }
        states_.push_back(ret);
      }
      unsigned int i=0;
      for (; i < classes.size(); ++i) {
        classes[i].value.advance();
        if (classes[i].value.done()) {
          classes[i].value.reset();
        } else {
          break;
        }
      }
      if (i==classes.size()) break;
    }
    std::sort(states_.begin(), states_.end());
  }

  void DefaultSubsetStates::setup_classes(UF &equivalencies,
                                          const std::set<Particle*>& seen,
                                          Subset *s,
                                          ParticleStatesTable *table,
                                          std::vector<Class> &classes) {
    std::map<Particle*, int> indexes;
    unsigned int sz=s->get_number_of_particles();
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
        classes.push_back(Class());
        sets[t]=classes.size()-1;
        ParticleStates *e;
        e=table->get_particle_states(t);
        classes.back().n= e->get_number_of_states();
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
      IMP_INTERNAL_CHECK(classes.size() > thisclass,
                         "Can't deal with class " << thisclass
                         << " because it is larger than "
                         << classes.size());
      classes[thisclass].indexes.push_back(indexes[t]);
    }
    for (unsigned int i=0; i< classes.size(); ++i) {
      classes[i].value.initialize(classes[i].n,
                                  classes[i].indexes.size());
    }
  }

  unsigned int DefaultSubsetStates::get_number_of_states() const {
    return states_.size();
  }
  SubsetState DefaultSubsetStates::get_state(unsigned int i) const {
    return states_[i];
  }

  bool DefaultSubsetStates::get_is_state(const SubsetState &state) const {
    return std::binary_search(states_.begin(), states_.end(), state);
  }

  void DefaultSubsetStates::do_show(std::ostream &out) const{}

}


DefaultSubsetStatesTable::DefaultSubsetStatesTable(ParticleStatesTable *pst):
  pst_(pst){}


SubsetStates* DefaultSubsetStatesTable::get_subset_states(Subset*s) const {
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
  for (unsigned int i=0; i< s->get_number_of_particles(); ++i) {
    Particle *a= s->get_particle(i);
    for (unsigned int j=0; j< i; ++j) {
      Particle *b= s->get_particle(j);
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
  return new DefaultSubsetStates(equivalencies, seen, s,
                                 pst_, set_);
}

void DefaultSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
