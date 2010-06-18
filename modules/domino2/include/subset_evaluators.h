/**
 *  \file domino2/subset_evaluators.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_EVALUATORS_H
#define IMPDOMINO2_SUBSET_EVALUATORS_H

#include "particle_states.h"
#include "SubsetState.h"
#include "internal/restraint_evaluator.h"
#include "Subset.h"
#include "domino2_macros.h"
#include "domino2_config.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/Configuration.h>
#include <IMP/Model.h>
#include <IMP/macros.h>


IMPDOMINO2_BEGIN_NAMESPACE
class DominoSampler;


/** An instance of this type is created by the
    SubsetEvaluatorTable::get_subset_evaluator method().  It has one
    method of interest, get_score() which returns the score for a
    given of the Subset which was passed to the
    SubsetEvaluatorTable::get_subset_evaluator method() call which
    returned it.  */
class IMPDOMINO2EXPORT SubsetEvaluator: public Object {
public:
  SubsetEvaluator(std::string name= "SubsetEvaluator");
  virtual double get_score(const SubsetState& state) const=0;
  virtual ~SubsetEvaluator();
};

IMP_OBJECTS(SubsetEvaluator, SubsetEvaluators);


/** A class which produces SubsetEvaluator objects upon
    demand.*/
class IMPDOMINO2EXPORT SubsetEvaluatorTable: public Object {
  WeakPointer<const DominoSampler> sampler_;
  friend class DominoSampler;
 protected:
  const DominoSampler *get_sampler() const {
    return sampler_;
  }
 public:
#ifndef IMP_DOXYGEN
  void set_sampler(const DominoSampler *sampler) {
    sampler_=sampler;
  }
#endif
  virtual SubsetEvaluator* get_subset_evaluator(const Subset &s) const=0;
  virtual ~SubsetEvaluatorTable();
};

IMP_OBJECTS(SubsetEvaluatorTable, SubsetEvaluatorFactories);


//! Score a configuration of the subset using the Model
/** A given Subset is scored using all the restraints
    which do not have any optimized particles
    as input except ones in the subset.
 */
class IMPDOMINO2EXPORT ModelSubsetEvaluatorTable:
  public SubsetEvaluatorTable {
  internal::ModelData data_;
public:
  ModelSubsetEvaluatorTable(Model *m, ParticleStatesTable *pst);
  IMP_SUBSET_EVALUATOR_TABLE(ModelSubsetEvaluatorTable);
};

IMP_OBJECTS(ModelSubsetEvaluatorTable,
            ModelSubsetEvaluatorTableFactories);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_EVALUATORS_H */
