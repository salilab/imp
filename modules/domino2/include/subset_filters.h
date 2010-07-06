/**
 *  \file domino2/subset_filters.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_FILTERS_H
#define IMPDOMINO2_SUBSET_FILTERS_H

#include "particle_states.h"
#include "SubsetState.h"
#include "particle_states.h"
#include "subset_evaluators.h"
#include "internal/restraint_evaluator.h"
#include "Subset.h"
#include "domino2_macros.h"
#include "domino2_config.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/Sampler.h>
#include <IMP/Configuration.h>
#include <IMP/Model.h>
#include <IMP/macros.h>


IMPDOMINO2_BEGIN_NAMESPACE

/** An instance of this type is created by the
    SubsetFilterTable::get_subset_filter method().  It has one
    method of interest, get_is_ok() which true if the state
    passes the filter.*/
class IMPDOMINO2EXPORT SubsetFilter: public Object {
public:
  SubsetFilter(std::string name= "SubsetFilter");
  virtual bool get_is_ok(const SubsetState& state) const=0;
  virtual ~SubsetFilter();
};

IMP_OBJECTS(SubsetFilter, SubsetFilters);


/** A class which produces SubsetFilter objects upon
    demand. The passed prior_subsets list is subsets of
    s which have already been filtered. These objects are
    used to reject partial states which, in some way,
    aren't good enough to be passed on to the final solution.*/
class IMPDOMINO2EXPORT SubsetFilterTable: public Object {
 public:
  virtual SubsetFilter* get_subset_filter(const Subset &s,
                                          const Subsets &prior_subsets) const=0;
  virtual ~SubsetFilterTable();
};

IMP_OBJECTS(SubsetFilterTable, SubsetFilterTables);


//! Filter a configuration of the subset using the Model
/** The maximum scores allowed for each restraint are
    taken from the Sampler.
 */
class IMPDOMINO2EXPORT RestraintScoreSubsetFilterTable:
  public SubsetFilterTable {
  Pointer<ModelSubsetEvaluatorTable> mset_;
  double max_;
 public:
  RestraintScoreSubsetFilterTable(ModelSubsetEvaluatorTable *eval,
                                  const Sampler *s);
  IMP_SUBSET_FILTER_TABLE(RestraintScoreSubsetFilterTable);
};

IMP_OBJECTS(RestraintScoreSubsetFilterTable,
            RestraintScoreSubsetFilterTables);

/** \brief Do not allow two particles to be in the same state.

    If a ParticleStatesTable is passed, then two particles cannot
    be in the same state if they have the same ParticleStates,
    otherwise, if a ParticlePairs is passed then pairs found in the
    list are excluded.
 */
class IMPDOMINO2EXPORT PermutationSubsetFilterTable:
  public SubsetFilterTable {
  Pointer<ParticleStatesTable> pst_;
  const ParticlePairsTemp pairs_;
public:
  PermutationSubsetFilterTable(ParticleStatesTable *pst);
  PermutationSubsetFilterTable(const ParticlePairsTemp &pairs);
  IMP_SUBSET_FILTER_TABLE(PermutationSubsetFilterTable);
};

IMP_OBJECTS(PermutationSubsetFilterTable,
            PermutationSubsetFilterTables);


/** \brief Force two particles to be in the same state.

    If a ParticleStatesTable is passed, then two particles must
    be in the same state if they have the same ParticleStates,
    otherwise, if a ParticlePairs is passed then pairs found in the
    list are excluded.
 */
class IMPDOMINO2EXPORT EqualitySubsetFilterTable:
  public SubsetFilterTable {
  Pointer<ParticleStatesTable> pst_;
  const ParticlePairsTemp pairs_;
public:
  EqualitySubsetFilterTable(ParticleStatesTable *pst);
  EqualitySubsetFilterTable(const ParticlePairsTemp &pairs);
  IMP_SUBSET_FILTER_TABLE(EqualitySubsetFilterTable);
};

IMP_OBJECTS(EqualitySubsetFilterTable,
            EqualitySubsetFilterTables);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_FILTERS_H */
