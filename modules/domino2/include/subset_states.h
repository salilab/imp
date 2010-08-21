/**
 *  \file domino2/subset_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_SUBSET_STATES_H
#define IMPDOMINO2_SUBSET_SUBSET_STATES_H


#include "particle_states.h"
#include "subset_filters.h"
#include "SubsetState.h"
#include "Subset.h"
#include "domino2_config.h"
#include "domino2_macros.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <boost/pending/disjoint_sets.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPDOMINO2_BEGIN_NAMESPACE
class DominoSampler;


/** Allow enumeration of the states of a particular subset.
    Straight forward examples
    would just return the product of the number of states returned by
    the StateEnumerator for each of the particles, while a permutation
    based one would have methods to define equivalency sets and only return
    permutations of the states of these sets.
*/
class IMPDOMINO2EXPORT SubsetStates: public Object {
public:
  SubsetStates(std::string name="SubsetStates %1%"): Object(name){}
  virtual unsigned int get_number_of_states() const=0;
  /** i can be anything in [0, get_number_of_states())
   */
  virtual SubsetState get_state(unsigned int i) const=0;
  /** Return true if there is some i for which get_state()
      returns s. */
  virtual bool get_is_state(const SubsetState &s) const=0;
  virtual ~SubsetStates();
};

/** The base class for classes that create SubsetStates, one per
    subset. The main method of interest is get_subset_states()
    which returns a SubsetStates object which can enumerate
    the states of a provided subset.
*/
class IMPDOMINO2EXPORT SubsetStatesTable: public Object {
  WeakPointer<const DominoSampler> sampler_;
  friend class DominoSampler;
 protected:
  const DominoSampler *get_sampler() const {return sampler_;}
  const bool get_has_sampler() const {return sampler_;}
public:
#ifndef IMP_DOXYGEN
  void set_sampler(const DominoSampler *sampler) {
    sampler_=sampler;
  }
#endif
  SubsetStatesTable(std::string name= "SubsetStatesTable %1%"): Object(name){}
  virtual SubsetStates *get_subset_states(const Subset &s) const=0;
  ~SubsetStatesTable();
};



/** Enumerate states based on provided ParticleStates
    objects.

    The produced states are filtered using a variety of methods
    - no two particles which have the same ParticleStates object
    in the ParticleStatesTable can be assigned the same state.
    That is for a given Subset s and SubsetState ss, if
    ParticleStatesTable::get_particle_states(s[i])
    ==ParticleStatesTable::get_particle_states(s[j])
    then ss[i] != ss[j]

    - If a SubsetFilterTable objects are provided, the branch and bound
    is used to eliminate states using them.
*/
class IMPDOMINO2EXPORT BranchAndBoundSubsetStatesTable:
  public SubsetStatesTable {
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
/* MSVC/Sun gcc appears confused by a friend class in the anonymous namespace */
public:
  Pointer<ParticleStatesTable> pst_;
  SubsetFilterTables sft_;
#endif
 public:
  BranchAndBoundSubsetStatesTable(ParticleStatesTable* pst,
                                  const SubsetFilterTables &sft
                                  = SubsetFilterTables());
  IMP_SUBSET_STATES_TABLE(BranchAndBoundSubsetStatesTable);
};

typedef BranchAndBoundSubsetStatesTable DefaultSubsetStatesTable;


/** Store a list of SubsetState objects and return them on demand. To be
    used with the ListSubsetStatesTable.
    \untested{ListSubsetStates}
*/
class IMPDOMINO2EXPORT ListSubsetStates: public SubsetStates {
  std::vector<SubsetState> states_;
 public:
  ListSubsetStates(std::string name="ListSubsetStates %1%");
  void add_subset_state(const SubsetState& s);
  IMP_SUBSET_STATES(ListSubsetStates);
};

// yikes :-)
IMP_OBJECTS(ListSubsetStates, ListSubsetStatesList);


/** Store a map of SubsetStates objects and return them on demand. To be
    used with the SubsetStatesTable.
    \untested{ListSubsetStatesTable}
*/
class IMPDOMINO2EXPORT ListSubsetStatesTable: public SubsetStatesTable {
  std::map<Subset, Pointer<SubsetStates> > states_;
 public:
  ListSubsetStatesTable(std::string name="ListSubsetStatesTable %1%");
  void set_subset_states(const Subset &s, SubsetStates *lsc) {
    states_[s]=lsc;
  }
  IMP_SUBSET_STATES_TABLE(ListSubsetStatesTable);
};

// yikes :-)
IMP_OBJECTS(ListSubsetStatesTable, ListSubsetStatesTables);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_SUBSET_STATES_H */
