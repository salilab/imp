/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_TREE_INFERENCE_H
#define IMPDOMINO_INTERNAL_TREE_INFERENCE_H

#include "../domino_config.h"
#include "../utility.h"
#include "../subset_filters.h"
#include "inference_utility.h"


IMPDOMINO_BEGIN_NAMESPACE
class SubsetEvaluatorTable;
class SubsetStatesTable;
IMPDOMINO_END_NAMESPACE

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

IMPDOMINOEXPORT
SubsetStates get_best_conformations(const SubsetGraph &jt,
                                    int root,
                                    const Subset& all_particles,
                                    const SubsetFilterTables &filters,
                                    const SubsetStatesTable *states,
                                    ListSubsetFilterTable *lsft,
                                    InferenceStatistics &stats);


IMPDOMINO_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO_INTERNAL_TREE_INFERENCE_H */
