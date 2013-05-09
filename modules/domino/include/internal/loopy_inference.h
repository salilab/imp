/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H
#define IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H

#include <IMP/domino/domino_config.h>
#include "../subset_filters.h"

#if 0
IMPDOMINO_BEGIN_NAMESPACE
class SubsetEvaluatorTable;
class AssignmentsTable;
IMPDOMINO_END_NAMESPACE

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

IMPDOMINOEXPORT Assignments loopy_get_best_conformations(
    const SubsetGraph &sg, const Subset &all_particles,
    const SubsetFilterTables &filters, const AssignmentsTable *sst,
    unsigned int max);

IMPDOMINO_END_INTERNAL_NAMESPACE
#endif
#endif /* IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H */
