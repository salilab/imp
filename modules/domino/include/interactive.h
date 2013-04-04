/**
 *  \file IMP/domino/interactive.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_INTERACTIVE_H
#define IMPDOMINO_INTERACTIVE_H

#include <IMP/domino/domino_config.h>
#include "Assignment.h"
#include "Subset.h"
#include "subset_filters.h"
#include "assignment_containers.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/display/Writer.h>
#include <IMP/dependency_graph.h>
#include <IMP/statistics/metric_clustering.h>

#ifdef IMP_DOMINO_USE_IMP_RMF
#include <RMF/HDF5/Group.h>
#endif


IMPDOMINO_BEGIN_NAMESPACE


  //! Fill in assignments for a leaf
IMPDOMINOEXPORT void load_leaf_assignments(const Subset& subset,
                                           AssignmentsTable *at,
                                           AssignmentContainer *ac);

  //! Fill in assignments for an internal node
  /** The passed assignments, the ordering for the children is that of
      the node indexes for the children.
      \unstable{load_merged_assignments}
  */
IMPDOMINOEXPORT void load_merged_assignments(const Subset &first_subset,
                                             AssignmentContainer* first,
                                             const Subset &second_subset,
                                             AssignmentContainer* second,
                                             const SubsetFilterTablesTemp
                                             &filters,
                                             AssignmentContainer* ret,
                                             double max_error=0,
                                             ParticleStatesTable *pst=nullptr,
                                             const statistics::Metrics &metrics
                                             = statistics::Metrics(),
                                             unsigned int max_states
                                             =std::numeric_limits<int>::max());

 //! Sample from the merged assignments.
  /** The passed assignments, the ordering for the children is that of
      the node indexes for the children.

      It will try maximum_tries times to merge an assignment from the first
      set with a randomly chosen assignment from the second. Note that duplicate
      solutions may be present in the output and the output will never be
      guaranteed to be complete.

      \unstable{load_sampled_merged_assignments}
  */
IMPDOMINOEXPORT void
load_merged_assignments_random_order(const Subset &first_subset,
                                     AssignmentContainer* first,
                                     const Subset &second_subset,
                                     AssignmentContainer* second,
                                     const SubsetFilterTablesTemp
                                     &filters,
                                     unsigned int maximum_tries,
                                     AssignmentContainer* ret);


IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_INTERACTIVE_H */
