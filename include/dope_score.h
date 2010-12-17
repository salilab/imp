/**
 *  \file membrane/dope_score.h
 *  \brief Dope scoring
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPDOPE_SCORE_H
#define IMPDOPE_SCORE_H

#include "membrane_config.h"
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/StatisticalPairScore.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/file.h>
#include <limits>

IMPMEMBRANE_BEGIN_NAMESPACE

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

typedef core::StatisticalPairScore<DopeType, false> DopePairScore;

IMPMEMBRANEEXPORT void add_dope_score_data(atom::Hierarchy h);

/** @} */
IMPMEMBRANE_END_NAMESPACE

#endif /* IMPDOPE_SCORE_H */
