/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/BranchAndBoundSampler.h>
#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/utility.h>
#include <IMP/file.h>

IMPDOMINO_BEGIN_NAMESPACE

BranchAndBoundSampler::BranchAndBoundSampler(Model *m, ParticleStatesTable *pst,
                                             std::string name)
    : DiscreteSampler(m, pst, name) {}

BranchAndBoundSampler::BranchAndBoundSampler(Model *m, std::string name)
    : DiscreteSampler(m, new ParticleStatesTable(), name) {}

Assignments BranchAndBoundSampler::do_get_sample_assignments(
    const Subset &s) const {
  SubsetFilterTables sfts = DiscreteSampler::get_subset_filter_tables_to_use(
      RestraintsTemp(1, get_model()->get_root_restraint_set()),
      get_particle_states_table());
  IMP::base::OwnerPointer<AssignmentsTable> sst =
      DiscreteSampler::get_assignments_table_to_use(sfts);
  IMP_NEW(PackedAssignmentContainer, pac, ());
  sst->load_assignments(s, pac);
  return pac->get_assignments(IntRange(0, pac->get_number_of_assignments()));
}

IMPDOMINO_END_NAMESPACE
