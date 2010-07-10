/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/BranchAndBoundSampler.h>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/utility.h>

#include <IMP/domino2/internal/inference.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


BranchAndBoundSampler::BranchAndBoundSampler(Model *m,
                                             ParticleStatesTable* pst):
  DiscreteSampler(m, pst, "BranchAndBound Sampler %1"){
}

BranchAndBoundSampler::BranchAndBoundSampler(Model *m):
  DiscreteSampler(m, new ParticleStatesTable(), "BranchAndBound Sampler %1")
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
  SubsetStatesList ret(ss->get_number_of_states());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= ss->get_state(i);
  }
  return ret;
}

void BranchAndBoundSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
