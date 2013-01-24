/**
 *  \file IMP/score_functor/Dope.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_DOPE_H
#define IMPSCORE_FUNCTOR_DOPE_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/base/file.h>
#include "Statistical.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE

/** The type of atoms for Dope.*/
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  IMP_DECLARE_CONTROLLED_KEY_TYPE(DopeType, 6453462);
#elif defined(SWIG)
class DopeType;
#endif

/**  Score pair of atoms based on DOPE.

  See M.-y. Shen and A. Sali. Statistical potential for assessment and
  prediction of protein structures. Protein Science 15, 2507–2524, 2006.

  DOPE should not be applied to two atoms from the same residue.
  You may need to use the SameResiduePairFilter to filter these out.

  You need to use IMP::atom::add_dope_score_data() to add the requisite
  atom type data to the particles being scored.
*/
class Dope: public Statistical<DopeType, false, true> {
  typedef  Statistical<DopeType, false, true> P;
public:
  Dope(double threshold  = std::numeric_limits<double>::max()):
    P(get_dope_type_key(), threshold, get_data_path("dope_score.lib")){}
  Dope(double threshold,
       base::TextInput data_file):  P(get_dope_type_key(), threshold,
                                      data_file){}
  static IntKey get_dope_type_key() {
    static const IntKey ik("dope atom type");
    return ik;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif  /* IMPSCORE_FUNCTOR_DOPE_H */
