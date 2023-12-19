/**
 *  \file internal/tree_inference.h
 *  \brief Utility functions for tree inference.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_TREE_INFERENCE_H
#define IMPDOMINO_INTERNAL_TREE_INFERENCE_H

#include <IMP/domino/domino_config.h>
#include "../utility.h"
#include "../subset_filters.h"
#include "inference_utility.h"

IMPDOMINO_BEGIN_NAMESPACE
class SubsetEvaluatorTable;
class AssignmentsTable;
IMPDOMINO_END_NAMESPACE

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

IMPDOMINOEXPORT void load_merged_assignments(
    const Subset &first_subset, AssignmentContainer *first,
    const Subset &second_subset, AssignmentContainer *second,
    const SubsetFilterTables &filters, ListSubsetFilterTable *lsft,
    InferenceStatistics *stats, size_t max_states, AssignmentContainer *out);

IMPDOMINOEXPORT void load_leaf_assignments(const Subset &merged_subset,
                                           const AssignmentsTable *states,
                                           ListSubsetFilterTable *lsft,
                                           InferenceStatistics *stats,
                                           AssignmentContainer *out);

IMPDOMINOEXPORT void load_best_conformations(
    const MergeTree &jt, int root, const Subset &all_particles,
    const SubsetFilterTables &filters, const AssignmentsTable *states,
    ListSubsetFilterTable *lsft, InferenceStatistics *stats, unsigned int max,
    AssignmentContainer *out);

IMPDOMINO_END_INTERNAL_NAMESPACE

#endif /* IMPDOMINO_INTERNAL_TREE_INFERENCE_H */
