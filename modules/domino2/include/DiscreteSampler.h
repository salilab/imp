/**
 *  \file domino2/DiscreteSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_DISCRETE_SAMPLER_H
#define IMPDOMINO2_DISCRETE_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "subset_states.h"
#include "particle_states.h"
#include "subset_evaluators.h"
#include "subset_filters.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE



//! A base class for discrete samplers in Domino2
/** All the samplers derived from it share some common properties:
    - each particle is allowed to have one of a discrete set of
    conformations, as stored in a ParticleStatesTable
    - there is, optionally, one scoring function, accessed through
    the SubsetEvaluatorTable
    - there can be many SubsetFilterTable objects which are
    used to prune the discrete space.

    Defaults are provided for all the parameters:
    - if no SubsetEvaluatorTable is provided, then the
    ModelSubsetEvaluatorTable is used.
    - if no SubsetFilterTables are provided, then the
    PermutationSubsetFilterTable and the
    RestraintScoreSubsetFilterTable are used.

    \note the restraint scores must be non-negative in general.
    If you are using restraints which can produce negative values,
    we can provide a restraint which wraps another and makes
    it non-negative. Ping us.
 */
class IMPDOMINO2EXPORT DiscreteSampler : public Sampler
{
  IMP::internal::OwnerPointer<ParticleStatesTable> pst_;
  IMP::internal::OwnerPointer<SubsetStatesTable> sst_;
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> set_;
#ifndef IMP_DOXYGEN
 protected:
  SubsetEvaluatorTable* get_subset_evaluator_table_to_use() const;
  SubsetFilterTables
    get_subset_filter_tables_to_use(SubsetEvaluatorTable *set) const;
  SubsetStatesTable*
    get_subset_states_table_to_use(const SubsetFilterTables &sfts) const;
#endif
public:
  DiscreteSampler(Model*m, ParticleStatesTable *pst, std::string name);
  /** Particle states can be set either using this method,
      or equivalently, by accessing the table itself
      using get_particle_states_table(). This method
      is provided for users who want to use the default values
      and want a simple inferface.*/
  void set_particle_states(Particle *p, ParticleStates *se) {
    pst_->set_particle_states(p, se);
  }

  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
      @{
   */
  void set_subset_evaluator_table(SubsetEvaluatorTable *eval) {
    set_=eval;
  }
  void set_particle_states_table(ParticleStatesTable *cse) {
    pst_= cse;
  }
  void set_subset_states_table(SubsetStatesTable *sst) {
    sst_=sst;
  }
  SubsetEvaluatorTable* get_subset_evaluator_table() const {
    return set_;
  }
  ParticleStatesTable* get_particle_states_table() const {
    return pst_;
  }
  IMP_LIST(public, SubsetFilterTable, subset_filter_table,
           SubsetFilterTable*, SubsetFilterTables);
  /** @} */
};


IMP_OBJECTS(DiscreteSampler, DiscreteSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_DISCRETE_SAMPLER_H */
