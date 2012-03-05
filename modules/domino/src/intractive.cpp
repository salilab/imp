/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/interactive.h>
#include <IMP/domino/internal/tree_inference.h>


IMPDOMINO_BEGIN_NAMESPACE





void load_leaf_assignments(const Subset& subset,
                             AssignmentsTable *at,
                             AssignmentContainer *ac) {
  IMP_FUNCTION_LOG;
  internal::load_leaf_assignments(subset,
                                  at, nullptr, NULL,
                                  ac);
}


void load_merged_assignments(const Subset &first_subset,
                             AssignmentContainer* first,
                             const Subset &second_subset,
                             AssignmentContainer* second,
                             const SubsetFilterTablesTemp &filters,
                             AssignmentContainer* ret,
                             double max_error,
                             ParticleStatesTable *pst,
                             const statistics::Metrics &metrics,
                             unsigned int max_states) {
  IMP_FUNCTION_LOG;
  SubsetFilterTables ts(filters.begin(), filters.end());
  internal::load_merged_assignments(first_subset, first,
                                    second_subset, second,
                                    ts, nullptr, NULL,
                                    max_error, pst, metrics,
                                    max_states, ret);
}


IMPDOMINO_END_NAMESPACE
