/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/BranchAndBoundSampler.h>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/utility.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


BranchAndBoundSampler::BranchAndBoundSampler(Model *m,
                                             ParticleStatesTable* pst,
                                             std::string name):
  DiscreteSampler(m, pst, name){
}

BranchAndBoundSampler::BranchAndBoundSampler(Model *m,
                                             std::string name):
  DiscreteSampler(m, new ParticleStatesTable(), name)
{
}



SubsetStatesList BranchAndBoundSampler
::do_get_sample_states(const Subset &s) const {
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> set
    = DiscreteSampler::get_subset_evaluator_table_to_use();
  SubsetFilterTables sfts
    = DiscreteSampler::get_subset_filter_tables_to_use(set);
  IMP::internal::OwnerPointer<SubsetStatesTable> sst
    = DiscreteSampler::get_subset_states_table_to_use(sfts);
  IMP::internal::OwnerPointer<const SubsetStates> ss
    = sst->get_subset_states(s);
  SubsetStatesList ret=ss->get_subset_states();
  return ret;
}

void BranchAndBoundSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
