/**
 *  \file IMP/atom/LoopStatisticalPairScore.h
 *  \brief Fiser/Melo loop modeling statistical potential
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_LOOP_STATISTICAL_PAIR_SCORE_H
#define IMPATOM_LOOP_STATISTICAL_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/score_functor/LoopStatistical.h>
#include <IMP/score_functor/DistancePairScore.h>
#include <IMP/core/StatisticalPairScore.h>

IMPATOM_BEGIN_NAMESPACE

/** \name Fiser/Melo loop modeling statistical potential

    \imp provides the Fiser/Melo loop modeling statistical potential.

    \note These are quite large objects as they store the whole
    lookup table. It is much better to share them between
    restraints than to create separate instances.

 */
class LoopStatisticalPairScore;
typedef score_functor::LoopStatisticalType LoopStatisticalType;

//! Score atoms based on the Fiser/Melo loop modeling statistical potential
/**
    See Fiser et al, Modeling of loops in protein structures.
    Protein Science 9, 1753-1773, 2000.

    Like DOPE, this potential should not be applied to two atoms from
    the same residue. You may need to use the SameResiduePairFilter
    to filter these out.
*/
class LoopStatisticalPairScore
    : public score_functor::DistancePairScore<score_functor::LoopStatistical> {
  typedef score_functor::DistancePairScore<score_functor::LoopStatistical> P;

 public:
  LoopStatisticalPairScore(
    double threshold = std::numeric_limits<double>::max())
      : P(score_functor::LoopStatistical(threshold)) {}
  LoopStatisticalPairScore(double threshold, TextInput data_file)
      : P(score_functor::LoopStatistical(threshold, data_file)) {}
};

/** Add the Fiser/Melo loop modeling atom types to the atoms in the hierarchy.
 */
IMPATOMEXPORT void add_loop_statistical_score_data(atom::Hierarchy h);

/** @} */
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LOOP_STATISTICAL_PAIR_SCORE_H */
