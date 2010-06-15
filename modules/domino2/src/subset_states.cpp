/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
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
                           typename std::iterator_traits<It>::value_type end) {
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
                            typename std::iterator_traits<It>::value_type end) {
    return *b==-1;
  }


  class  DefaultSubsetStates: public SubsetStates {
  public:
    struct Class {
      Class(): n(0), k(0){}
      template <class It>
      Class(unsigned int ni,
            It b, It e): n(ni), k(std::distance(b,e)),
                         indexes(new int[k]),
                         permutation(new int[k]){
        std::copy(b,e,indexes.get());
        permutation_initialize(permutation.get(),
                               permutation.get()+k,
                               0, n);
      }
      unsigned int n;
      unsigned int k;
      boost::scoped_array<int> indexes;
      boost::scoped_array<int> permutation;
      void copy_from(const Class &o) {
        n=o.n;
        k=o.k;
        if (k > 0) {
          indexes.reset(new int[k]);
          permutation.reset(new int[k]);
          for (unsigned int i=0; i< k; ++i) {
            indexes[i]= o.indexes[i];
            permutation[i]= o.permutation[i];
          }
        }
      }
      IMP_COPY_CONSTRUCTOR(Class);
    };
    void setup_classes(UF &equivalencies,
                       const std::set<Particle*>& seen,
                       Subset *s,
                       ParticleStatesTable *table,
                       std::vector<Class> &classes);
    void reset_state() const;
    void advance_state() const;
    std::vector<SubsetState> states_;
    double max_;
    DefaultSubsetStates(UF &equivalencies,
                        const std::set<Particle*> &seen,
                        double max,
                        Subset *s,
                        ParticleStatesTable *table,
                        SubsetEvaluatorTable *set);
    typedef std::pair<boost::array<int, 2>,
                      Pointer<SubsetEvaluator> > EvaluatorPair;
    typedef std::vector< std::vector<EvaluatorPair> > PrefixEvaluators;
    typedef std::vector<PrefixEvaluators> PrefixEvaluatorsList;
    void fill_prefix_evaluators(const std::vector<Class> &classes,
                                Subset *s,
                                SubsetEvaluatorTable *set,
                                PrefixEvaluatorsList &pre);
    bool filter(const SubsetState &is,
                const std::vector<EvaluatorPair> &ses);
    IMP_SUBSET_STATES(DefaultSubsetStates);
  };

 void DefaultSubsetStates::
 fill_prefix_evaluators(const std::vector<Class> &classes,
                        Subset *s,
                        SubsetEvaluatorTable *set,
                        PrefixEvaluatorsList &ses) {
   ses.resize(classes.size());
    SubsetState is(s->get_number_of_particles());
    Ints prior;
    for (unsigned int i=0; i< classes.size(); ++i) {
      for (unsigned int j=0; j< classes[i].k; ++j) {
        ses[j].resize(classes[i].k);
        for (unsigned int k=0; k < classes[i].k; ++k) {
          for (unsigned int l=0; l < k; ++l) {
            boost::array<int,2> ids;
            ids[0]=classes[i].indexes[k];
            ids[1]=classes[i].indexes[l];
            ParticlesTemp ps(2);
            ps[0]=s->get_particle(ids[0]);
            ps[1]=s->get_particle(ids[1]);
            IMP_NEW(Subset, cus, (ps));
            std::cout << "creating in class evaluator for " << ids[0] << " "
                      << ids[1] << std::endl;
            ses[i][j].push_back(EvaluatorPair(ids,
                              set->get_subset_evaluator(cus)));
          }
          for (unsigned int l=0; l < prior.size(); ++l) {
            boost::array<int,2> ids;
            ids[0]=prior[l];
            ids[1]=classes[i].indexes[k];
            ParticlesTemp ps(2);
            ps[0]=s->get_particle(ids[0]);
            ps[1]=s->get_particle(ids[1]);
            IMP_NEW(Subset, cus, (ps));
            std::cout << "creating evaluator for " << ids[0] << " "
                      << ids[1] << std::endl;
            ses[i][j].push_back(EvaluatorPair(ids,
                            set->get_subset_evaluator(cus)));
          }
        }
      }
      for (unsigned int j=0; j< classes[i].k; ++j) {
        prior.push_back(classes[i].indexes[j]);
      }
    }
  }

  bool DefaultSubsetStates::filter(const SubsetState &is,
                                   const std::vector<EvaluatorPair> &ses) {
    for (unsigned int j=0; j< ses.size(); ++j) {
      SubsetState ss(2);
      ss[0]= is[ses[j].first[0]];
      ss[1]= is[ses[j].first[1]];
      double score= ses[j].second->get_score(ss);
      if (score > max_) {
        return false;
      }
    }
    return true;
  }

  DefaultSubsetStates::DefaultSubsetStates(UF &equivalencies,
                                           const std::set<Particle*>& seen,
                                           double max,
                                           Subset *s,
                                           ParticleStatesTable *table,
                                           SubsetEvaluatorTable *set):
    SubsetStates("DefaultSubsetStates on "+s->get_name()), max_(max) {
    std::vector<Class> classes;
    IMP_CHECK_OBJECT(table);
    IMP_CHECK_OBJECT(s);
    setup_classes(equivalencies, seen, s, table, classes);

    for (unsigned int i=0; i< classes.size(); ++i) {
      permutation_initialize(classes[i].permutation.get(),
                             classes[i].permutation.get()+classes[i].k,
                             0, classes[i].n);
    }
    const unsigned int sz=s->get_number_of_particles();
    PrefixEvaluatorsList ses;
    if (set) {
      fill_prefix_evaluators(classes, s, set, ses);
    }
    SubsetState ret(sz);
    for (unsigned int i=0; i < classes.size(); ++i) {
      for (unsigned int j=0; j< classes[i].k; ++j) {
        ret[classes[i].indexes[j]] = classes[i].permutation[j];
      }
    }
    states_.push_back(ret);
    while (true) {
      bool overflow=true;
      for (int i=0; i < static_cast<int>(classes.size()) && overflow; ++i) {
        overflow=permutation_advance(classes[i].permutation.get(),
                                     classes[i].permutation.get()+classes[i].k,
                                     0, classes[i].n);
        for (unsigned int j=0; j< classes[i].k; ++j) {
          ret[classes[i].indexes[j]] = classes[i].permutation[j];
          if (set) {
            bool ok= filter(ret, ses[i][j]);
            if (!ok) {
              permutation_advance_i(classes[i].permutation.get(),
                                    classes[i].permutation.get()+classes[i].k,
                                    0, classes[i].n, j);
              std::cout << "Filtered " << ret << std::endl;
              --i;
              overflow=true;
              break;
            }
          }
        }
      }
      if (overflow) break;
      else {
        std::cout << "pushing " << ret << std::endl;
        states_.push_back(ret);
      }
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
    std::vector<Ints> values;
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
        values.push_back(Ints());
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
      values[thisclass].push_back(indexes[t]);
    }
    for (unsigned int i=0; i< classes.size(); ++i) {
      classes[i]= Class(classes[i].n, values[i].begin(), values[i].end());
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
  double max;
  if (get_has_sampler()) {
    max= get_sampler()->get_maximum_score();
  } else {
    max= std::numeric_limits<double>::max();
  }
  return new DefaultSubsetStates(equivalencies, seen, max,
                                 s,
                                 pst_, set_);
}

void DefaultSubsetStatesTable::do_show(std::ostream &out) const {
}


IMPDOMINO2_END_NAMESPACE
