/**
 *  \file domino2/subset_evaluators.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_EVALUATORS_H
#define IMPDOMINO2_SUBSET_EVALUATORS_H

#include "domino2_config.h"
#include "domino2_macros.h"
#include "particle_states.h"
#include "SubsetState.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/Configuration.h>
#include <IMP/Model.h>
#include <IMP/macros.h>

IMPDOMINO2_BEGIN_NAMESPACE
class DominoSampler;


/** Return the score for a state defined by the subset of particles in
    the given enumerated states. The table is provided so that they can
    be calculated if they are not pre-computed.
*/
class IMPDOMINO2EXPORT SubsetEvaluator: public Object {
public:
  SubsetEvaluator(std::string name= "SubsetEvaluator");
  virtual double get_score(const SubsetState& state) const=0;
  virtual ~SubsetEvaluator();
};

IMP_OBJECTS(SubsetEvaluator, SubsetEvaluators);

class IMPDOMINO2EXPORT SubsetEvaluatorTable: public Object {
  WeakPointer<const DominoSampler> sampler_;
  friend class DominoSampler;
  void set_sampler(const DominoSampler *sampler) {
    sampler_=sampler;
  }
 public:
  virtual SubsetEvaluator* get_subset_evaluator(Subset *s) const=0;
  virtual ~SubsetEvaluatorTable();
};

IMP_OBJECTS(SubsetEvaluatorTable, SubsetEvaluatorFactories);

//! An evaluator which loads the state into the model and calls model evaluate
/** When the object is created, the state of the model is saved. This saved
    model state is used to initialize the particles which are not part of the
    current state. Incremental evaluation is used to avoid spending too
    much time computing scores for the irrelevant particles.
*/
class IMPDOMINO2EXPORT ModelSubsetEvaluatorTable: public SubsetEvaluatorTable {
  mutable Pointer<Model> model_;
  IMP::internal::OwnerPointer<Configuration> cs_;
  Pointer<ParticleStatesTable> pst_;
  mutable std::map<Particle*, ParticlesTemp> dependents_;
public:
  ModelSubsetEvaluatorTable(Model *m, ParticleStatesTable *pst);
  IMP_SUBSET_EVALUATOR_TABLE(ModelSubsetEvaluatorTable);
};

IMP_OBJECTS(ModelSubsetEvaluatorTable, ModelSubsetEvaluatorFactories);

#if 0
//! This one evaluates the restraints one at a time and caches the result
class IMPDOMINO2EXPORT CachingModelSubsetEvaluatorTable:
  public SubsetEvaluatorTable {
  mutable std::map<Particle*, ParticlesTemp> dependents_;
  Pointer<internal::CachingEvaluatorData> data_;
public:
  CachingModelSubsetEvaluatorTable(Model *m, ParticleStatesTable *pst);
  IMP_SUBSET_EVALUATOR_TABLE(CachingModelSubsetEvaluatorTable);
};

IMP_OBJECTS(CachingModelSubsetEvaluatorTable,
            CachingModelSubsetEvaluatorTableFactories);
#endif

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_EVALUATORS_H */
