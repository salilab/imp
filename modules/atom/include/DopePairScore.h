/**
 *  \file atom/DopePairScore.h
 *  \brief Dope scoring
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPATOM_DOPE_PAIR_SCORE_H
#define IMPATOM_DOPE_PAIR_SCORE_H

#include "atom_config.h"
#include "Hierarchy.h"
#include <IMP/core/StatisticalPairScore.h>

IMPATOM_BEGIN_NAMESPACE

/** \name Dope scoring

    \imp provides the DOPE scoring function for scoring proteins

 */
class DopePairScore;

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  IMP_DECLARE_CONTROLLED_KEY_TYPE(DopeType, 6453462);
#elif defined(SWIG)
class DopeType;
#endif

/**
  Score pair of atoms based on DOPE.

  See M.-y. Shen and A. Sali. Statistical potential for assessment and
  prediction of protein structures. Protein Science 15, 2507–2524, 2006.

  Dope should not be applied to two atoms from the same residue.
  You may need to use the SameResiduePairFilter to filter these out.
*/
class IMPATOMEXPORT DopePairScore:
  public core::StatisticalPairScore<DopeType, false, true> {
  typedef core::StatisticalPairScore<DopeType, false, true>  P;
 public:
  DopePairScore(double threshold
                             = std::numeric_limits<double>::max());
  DopePairScore(double threshold,
                TextInput data_file);
};

/** Add the dope atom types to the atoms in the hierarchy.
 */
IMPATOMEXPORT void add_dope_score_data(atom::Hierarchy h);

/** @} */
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DOPE_PAIR_SCORE_H */
