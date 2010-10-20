/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_DOMINO_SAMPLER_H
#define IMPDOMINO2_DOMINO_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "DiscreteSampler.h"
#include "subset_graphs.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE
#ifdef SWIG
class SubsetGraph;
#endif

//! Sample best solutions using Domino
/**
 */
class IMPDOMINO2EXPORT DominoSampler : public DiscreteSampler
{
  Pointer<SubsetStatesTable> sst_;
  SubsetGraph sg_;
  bool has_sg_;
  bool csf_;
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
  void set_subset_graph(const SubsetGraph &sg);
  /** @} */

  /** perform filtering between subsets based by eliminating states
      that cannot be realized in a subset.
  */
  void set_use_cross_subset_filtering(bool tf) {
    csf_=tf;
  }
};


IMP_OBJECTS(DominoSampler, DominoSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_DOMINO_SAMPLER_H */
