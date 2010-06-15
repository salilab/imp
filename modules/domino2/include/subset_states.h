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
#include "subset_evaluators.h"
#include "SubsetState.h"
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
  SubsetStates(std::string name="SubsetStates"): Object(name){}
  virtual unsigned int get_number_of_states() const=0;
  virtual SubsetState get_state(unsigned int i) const=0;
  virtual bool get_is_state(const SubsetState &s) const=0;
  virtual ~SubsetStates();
};

/** The base class for classes that create SubsetStates, one per
    subset. The sampler calls set_particle_states_table() when the
    factory is added to the sampler.
*/
class IMPDOMINO2EXPORT SubsetStatesTable: public Object {
  WeakPointer<const DominoSampler> sampler_;
  friend class DominoSampler;
  void set_sampler(const DominoSampler *sampler) {
    sampler_=sampler;
  }
 protected:
  const DominoSampler *get_sampler() const {return sampler_;}
  const bool get_has_sampler() const {return sampler_;}
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
    circumstances). Equivalency classes are automatically determined
    from the ParticleStates objects in the passed table.

    An (optional) subset evaluator can be provided, which will be
    used to filter subsets based on their score and the bounds
    provided in the sampler.
*/
class IMPDOMINO2EXPORT DefaultSubsetStatesTable: public SubsetStatesTable {
  friend class DefaultSubsetStates;
  Pointer<ParticleStatesTable> pst_;
  Pointer<SubsetEvaluatorTable> set_;
 public:
  DefaultSubsetStatesTable(ParticleStatesTable* pst);
  void set_subset_evaluator_table(SubsetEvaluatorTable *set) {
    set_=set;
  }
  IMP_SUBSET_STATES_TABLE(DefaultSubsetStatesTable);
};


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_SUBSET_STATES_H */
