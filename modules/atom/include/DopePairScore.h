/**
 *  \file IMP/atom/DopePairScore.h
 *  \brief Dope scoring
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DOPE_PAIR_SCORE_H
#define IMPATOM_DOPE_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/score_functor/Dope.h>
#include <IMP/score_functor/DistancePairScore.h>
#include <IMP/core/StatisticalPairScore.h>

IMPATOM_BEGIN_NAMESPACE

/** \name Dope scoring

    \imp provides the DOPE scoring function for scoring proteins.

    \note These are quite large objects as they store the whole
    DOPE lookup table. It is much better to share them between
    restraints than to create separate instances.

 */
class DopePairScore;
typedef score_functor::DopeType DopeType;

/**
  Score pair of atoms based on DOPE.

  See M.-y. Shen and A. Sali. Statistical potential for assessment and
  prediction of protein structures. Protein Science 15, 2507–2524, 2006.

  DOPE should not be applied to two atoms from the same residue.
  You may need to use the SameResiduePairFilter to filter these out.
*/
class DopePairScore:
  public score_functor::DistancePairScore<score_functor::Dope> {
  typedef score_functor::DistancePairScore<score_functor::Dope>  P;
 public:
  DopePairScore(double threshold
                = std::numeric_limits<double>::max()):
    P(score_functor::Dope(threshold)){}
  DopePairScore(double threshold,
                base::TextInput data_file):
    P(score_functor::Dope(threshold, data_file)){}
};

/** Add the dope atom types to the atoms in the hierarchy.
 */
IMPATOMEXPORT void add_dope_score_data(atom::Hierarchy h);

/** @} */
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DOPE_PAIR_SCORE_H */
