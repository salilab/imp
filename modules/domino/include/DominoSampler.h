/**
 *  \file IMP/domino/DominoSampler.h
 *  \brief A Bayesian inference-based sampler.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_DOMINO_SAMPLER_H
#define IMPDOMINO_DOMINO_SAMPLER_H

#include <IMP/domino/domino_config.h>
//#include "Evaluator.h"
#include "DiscreteSampler.h"
#include "subset_graphs.h"
#include "internal/inference_utility.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/base/Pointer.h>

IMPDOMINO_BEGIN_NAMESPACE
#ifdef SWIG
class SubsetGraph;
#endif

//! Sample best solutions using Domino
/** Note that if there are many solutions, the ConfigurationSet returned
    by get_sample() might be huge (in terms of memory usage) and slow to
    generate. The Assignments returned by get_sample_assignments() can be
    a lot smaller and faster.
 */
class IMPDOMINOEXPORT DominoSampler : public DiscreteSampler {
  base::Pointer<AssignmentsTable> sst_;
  SubsetGraph sg_;
  MergeTree mt_;
  bool has_mt_;
  bool csf_;
  mutable internal::InferenceStatistics stats_;

 public:
  DominoSampler(kernel::Model *m, std::string name = "DominoSampler %1%");
  DominoSampler(kernel::Model *m, ParticleStatesTable *pst,
                std::string name = "DominoSampler %1%");
  Assignments do_get_sample_assignments(const IMP::domino::Subset &known) const
      IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DominoSampler);

 public:
  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
      @{
  */
  //! Specify the merge tree directly
  void set_merge_tree(const MergeTree &mt);
  /** @} */

  /** perform filtering between subsets based by eliminating states
      that cannot be realized in a subset.
  */
  void set_use_cross_subset_filtering(bool tf) { csf_ = tf; }

  /** \name Statistics
      If you specify the merge tree explicitly, you can query
      for statistics about particular nodes in the merge tree.
      @{
  */
  //! Get the number of states found for the merge at that vertex of the tree
  unsigned int get_number_of_assignments_for_vertex(unsigned int tree_vertex)
      const;
  //! Return a few subset states from that merge
  Assignments get_sample_assignments_for_vertex(unsigned int tree_vertex) const;
  /** @} */

  /** \name Interactive mode
      Once a merge tree is specified, one can interactively perform the
      sampling by requesting that domino fill in the Assignments for a
      given node of the merge tree from the assignments for the children.
      This can be useful for debugging Domino as well as for providing
      a distributed implementation.

      For each method, you can pass max_states to limit how many states are
      generated.

      You might want to just use load_leaf_assignments() and
      load_merged_assignments() instead.
      @{
  */
  //! Fill in assignments for a leaf
  Assignments get_vertex_assignments(unsigned int node_index,
                                     unsigned int max_states =
                                         std::numeric_limits<int>::max()) const;
  //! Fill in assignments for an internal node
  /** The passed assignments, the ordering for the children is
      the node index for the children.
  */
  Assignments get_vertex_assignments(unsigned int node_index,
                                     const Assignments &first,
                                     const Assignments &second,
                                     unsigned int max_states =
                                         std::numeric_limits<int>::max()) const;

  //! Fill in assignments for a leaf
  void load_vertex_assignments(unsigned int node_index, AssignmentContainer *ac,
                               unsigned int max_states =
                                   std::numeric_limits<int>::max()) const;
  //! Fill in assignments for an internal node
  /** The passed assignments, the ordering for the children is that of
      the node indexes for the children.
  */
  void load_vertex_assignments(unsigned int node_index,
                               AssignmentContainer *first,
                               AssignmentContainer *second,
                               AssignmentContainer *ac,
                               unsigned int max_states =
                                   std::numeric_limits<int>::max()) const;

  /** @} */
};

IMP_OBJECTS(DominoSampler, DominoSamplers);

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_DOMINO_SAMPLER_H */
