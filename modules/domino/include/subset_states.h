/**
 *  \file domino/subset_states.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_SUBSET_STATES_H
#define IMPDOMINO_SUBSET_SUBSET_STATES_H


#include "particle_states.h"
#include "subset_filters.h"
#include "SubsetState.h"
#include "Subset.h"
#include "domino_config.h"
#include "domino_macros.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/map.h>
#include <boost/pending/disjoint_sets.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPDOMINO_BEGIN_NAMESPACE
class DominoSampler;

/** The base class for classes that create SubsetStates, one per
    subset. The main method of interest is get_subset_states()
    which returns a SubsetStates containing the valid states.
*/
class IMPDOMINOEXPORT SubsetStatesTable: public Object {
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
  virtual SubsetStates get_subset_states(const Subset &s) const=0;
  ~SubsetStatesTable();
};

IMP_OBJECTS(SubsetStatesTable, SubsetStatesTables);


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
class IMPDOMINOEXPORT BranchAndBoundSubsetStatesTable:
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


/** Store a map of SubsetStates objects and return them on demand.
    \untested{ListSubsetStatesTable}
*/
class IMPDOMINOEXPORT ListSubsetStatesTable: public SubsetStatesTable {
  IMP::internal::Map<Subset, SubsetStates> states_;
 public:
  ListSubsetStatesTable(std::string name="ListSubsetStatesTable %1%");
  /** There must not be any duplicates in the list */
  void set_subset_states(const Subset &s, const SubsetStates &lsc) {
    IMP_IF_CHECK(USAGE) {
      SubsetStates l= lsc;
      std::sort(l.begin(), l.end());
      IMP_USAGE_CHECK(std::unique(l.begin(), l.end())== l.end(),
                      "There are duplicated subset states in the passed list");
    }
    states_[s]=lsc;
  }
  IMP_SUBSET_STATES_TABLE(ListSubsetStatesTable);
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SUBSET_SUBSET_STATES_H */
