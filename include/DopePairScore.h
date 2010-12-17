/**
 *  \file membrane/DopePairScore.h
 *  \brief Dope scoring
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPDOPE_PAIR_SCORE_H
#define IMPDOPE_PAIR_SCORE_H

#include "membrane_config.h"
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/StatisticalPairScore.h>

IMPMEMBRANE_BEGIN_NAMESPACE

/** \name Dope scoring

    \imp provides the DOPE scoring function for scoring proteins

 */
class DopePairScore;

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
  IMP_DECLARE_CONTROLLED_KEY_TYPE(DopeType, 6453462);
}
#elif defined(SWIG)
class DopeType;
#endif

/**

  Score pair of atoms based on DOPE

*/
class IMPMEMBRANEEXPORT DopePairScore:
  public core::StatisticalPairScore<DopeType, true> {
  typedef core::StatisticalPairScore<DopeType, true>  P;
 public:
  DopePairScore(double threshold
                             = std::numeric_limits<double>::max());
  DopePairScore(double threshold,
                             TextInput data_file);
};

IMP_OBJECTS(DopePairScore,DopePairScores);


IMPMEMBRANEEXPORT void add_dope_score_data(atom::Hierarchy h);

/** @} */
IMPMEMBRANE_END_NAMESPACE

#endif /* IMPDOPE_PAIR_SCORE_H */
