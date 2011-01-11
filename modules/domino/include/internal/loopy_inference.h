/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H
#define IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H

#include "../domino_config.h"
#include "../subset_filters.h"

IMPDOMINO_BEGIN_NAMESPACE
class SubsetEvaluatorTable;
class SubsetStatesTable;
IMPDOMINO_END_NAMESPACE

IMPDOMINO_BEGIN_INTERNAL_NAMESPACE

IMPDOMINOEXPORT
SubsetStates loopy_get_best_conformations(const SubsetGraph &sg,
                                          const Subset& all_particles,
                                          const SubsetFilterTables &filters,
                                              const SubsetStatesTable *sst);


IMPDOMINO_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO_INTERNAL_LOOPY_INFERENCE_H */
