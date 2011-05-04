/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_DOMINO_SAMPLER_H
#define IMPDOMINO_DOMINO_SAMPLER_H

#include "domino_config.h"
//#include "Evaluator.h"
#include "DiscreteSampler.h"
#include "subset_graphs.h"
#include "internal/inference_utility.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

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
class IMPDOMINOEXPORT DominoSampler : public DiscreteSampler
{
  Pointer<AssignmentsTable> sst_;
  SubsetGraph sg_;
  MergeTree mt_;
  bool has_sg_, has_mt_;
  bool csf_;
  mutable internal::InferenceStatistics stats_;
 public:
  DominoSampler(Model *m, std::string name= "DominoSampler %1%");
  DominoSampler(Model*m, ParticleStatesTable *pst,
                std::string name= "DominoSampler %1%");
  IMP_DISCRETE_SAMPLER(DominoSampler);
 public:
  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
      @{
  */
  //! Use "loopy" inference on a subset graph
  void set_subset_graph(const SubsetGraph &sg);
  //! Specify the merge tree directly
  void set_merge_tree(const MergeTree &mt);
  /** @} */

  /** perform filtering between subsets based by eliminating states
      that cannot be realized in a subset.
  */
  void set_use_cross_subset_filtering(bool tf) {
    csf_=tf;
  }

  /** \name Statistics
      If you specify the merge tree explicitly, you can query
      for statistics about particular nodes in the merge tree.
      @{
  */
  //! Get the number of states found for the merge at that vertex of the tree
  unsigned int
    get_number_of_assignments_for_vertex(unsigned int tree_vertex) const;
  //! Return a few subset states from that merge
  Assignments
    get_sample_assignments_for_vertex(unsigned int tree_vertex) const;
  /** @} */
};


IMP_OBJECTS(DominoSampler, DominoSamplers);


IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_DOMINO_SAMPLER_H */
