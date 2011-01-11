/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/BranchAndBoundSampler.h>
#include <IMP/domino/subset_states.h>
#include <IMP/domino/utility.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO_BEGIN_NAMESPACE


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



SubsetStates BranchAndBoundSampler
::do_get_sample_states(const Subset &s) const {
  SubsetFilterTables sfts
    = DiscreteSampler
    ::get_subset_filter_tables_to_use(get_model()->get_root_restraint_set(),
                                      get_particle_states_table());
  IMP::internal::OwnerPointer<SubsetStatesTable> sst
    = DiscreteSampler::get_subset_states_table_to_use(sfts);
  const SubsetStates ret  = sst->get_subset_states(s);
  return ret;
}

void BranchAndBoundSampler::do_show(std::ostream &out) const {
}

IMPDOMINO_END_NAMESPACE
