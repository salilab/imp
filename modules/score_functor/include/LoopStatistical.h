/**
 *  \file IMP/score_functor/LoopStatistical.h
 *  \brief Fiser/Melo loop modeling statistical potential
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSCORE_FUNCTOR_LOOP_STATISTICAL_H
#define IMPSCORE_FUNCTOR_LOOP_STATISTICAL_H

#include <IMP/score_functor/score_functor_config.h>
#include "Score.h"
#include <IMP/file.h>
#include "Statistical.h"
#include <IMP/algebra/utility.h>
IMPSCOREFUNCTOR_BEGIN_NAMESPACE
#ifdef SWIG
class LoopStatisticalType;
#else
/** The type of atoms for the loop modeling potential.*/
typedef Key<6453472> LoopStatisticalType;
IMP_VALUES(LoopStatisticalType, LoopStatisticalTypes);
#endif

//! Score atoms based on the Fiser/Melo loop modeling statistical potential
/**
    See Fiser et al, Modeling of loops in protein structures.
    Protein Science 9, 1753-1773, 2000.

    Like DOPE, this potential should not be applied to two atoms from
    the same residue. You may need to use the SameResiduePairFilter
    to filter these out.

    You need to use IMP::atom::add_loop_statistical_score_data()
    to add the requisite type data to the particles being scored.
*/
class LoopStatistical : public Statistical<LoopStatisticalType, false, true> {
  typedef Statistical<LoopStatisticalType, false, true> P;

 public:
  LoopStatistical(double threshold = std::numeric_limits<double>::max())
      : P(get_loop_type_key(), threshold, get_data_path("melo_score.lib")) {}
  LoopStatistical(double threshold, TextInput data_file)
      : P(get_loop_type_key(), threshold, data_file) {}
  static IntKey get_loop_type_key() {
    static const IntKey ik("loop statistical atom type");
    return ik;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_LOOP_STATISTICAL_H */
