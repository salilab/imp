/**
 *  \file domino2/subset_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_SUBSET_STATES_H
#define IMPDOMINO2_SUBSET_SUBSET_STATES_H

#include "domino2_config.h"
#include "domino2_macros.h"
#include "particle_states.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <boost/pending/disjoint_sets.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPDOMINO2_BEGIN_NAMESPACE
/** Store the configuration of a subset.*/
class SubsetState {
  std::vector<int> v_;
  int compare(const SubsetState &o) const {
    IMP_USAGE_CHECK(o.size() == size(), "Sizes don't match");
    for (unsigned int i=0; i< size(); ++i) {
      if (v_[i] < o[i]) return -1;
      else if (v_[i] > o[i]) return 1;
    }
    return 0;
  }
public:
typedef SubsetState This;
  SubsetState(){}
  SubsetState(unsigned int sz): v_(sz, -1){}
  IMP_COMPARISONS;
#ifndef SWIG
  int operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < v_.size(), "Out of range");
    IMP_USAGE_CHECK(v_[i] >=0, "Not initialized properly");
    return v_[i];
  }
  int& operator[](unsigned int i) {
    IMP_USAGE_CHECK(i < v_.size(), "Out of range");
    return v_[i];
  }
#endif
#ifndef IMP_DOXYGEN
  int __get__(unsigned int i) const {return operator[](i);}
  void __set__(unsigned int i, unsigned int v) {operator[](i)=v;}
#endif
  unsigned int size() const {
    return v_.size();
  }
  IMP_SHOWABLE_INLINE({
      out << "[";
      for (unsigned int i=0; i< size(); ++i) {
        out << v_[i];
        if (i != size()-1) out << " ";
      }
      out << "]";
    });
};

IMP_VALUES(SubsetState, SubsetStatesList);

IMP_OUTPUT_OPERATOR(SubsetState);

/** Allow enumeration of the states of a particular subset.
    Straight forward examples
    would just return the product of the number of states returned by
    the StateEnumerator for each of the particles, while a permutation
    based one would have methods to define equivalency sets and only return
    permutations of the states of these sets.
*/
class IMPDOMINO2EXPORT SubsetStates: public Object {
public:
  SubsetStates(std::string name="SubsetStates"): Object(name){}
  virtual unsigned int get_number_of_states() const=0;
  virtual SubsetState get_state(unsigned int i) const=0;
  virtual ~SubsetStates();
};

/** The base class for classes that create SubsetStates, one per
    subset. The sampler calls set_particle_states_table() when the
    factory is added to the sampler.
*/
class IMPDOMINO2EXPORT SubsetStatesTable: public Object {
public:
  SubsetStatesTable(std::string name= "SubsetStatesTable"): Object(name){}
  virtual SubsetStates *get_subset_states(Subset *s) const=0;
  ~SubsetStatesTable();
};



/** Enumerate states based on provided ParticleStates
    objects. "Equivalence classes" can be specified for particles,
    with the result that no two particles in the same class can be
    assigned the same state. That allows cheap avoidance of states
    where particles are assigned the same coordinates (in certain
    circumstances).
*/
class IMPDOMINO2EXPORT DefaultSubsetStatesTable: public SubsetStatesTable {
  friend class DefaultSubsetStates;
  typedef std::map<Particle*, Particle*> IParent;
  typedef std::map<Particle*, int> IRank;
  typedef boost::associative_property_map<IParent> Parent;
  typedef boost::associative_property_map<IRank > Rank;
  typedef boost::disjoint_sets<Rank, Parent> UF;
  Pointer<ParticleStatesTable> pst_;
  IParent parent_;
  IRank rank_;
  mutable UF equivalencies_;
  // for some reason boost disjoint sets doesn't provide a way to see
  // if an item is a set
  std::set<Particle*> seen_;
 public:
  DefaultSubsetStatesTable(ParticleStatesTable* pst);
  /** The two passed particles are treated as having equivalent
      and exclusive states. That is, particle a and particle b
      are not allowed to both be in state i, for any i. As a
      precondition, they must have the same StateEnumerator.
  */
  void add_equivalency(Particle *a, Particle* b);
  IMP_SUBSET_STATES_TABLE(DefaultSubsetStatesTable);
};


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_SUBSET_STATES_H */
