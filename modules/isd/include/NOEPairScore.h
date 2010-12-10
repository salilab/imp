/**
 *  \file NOEPairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_NOE_PAIR_SCORE_H
#define IMPISD_NOE_PAIR_SCORE_H

#include "isd_config.h"
#include <IMP/core/XYZ.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPISD_BEGIN_NAMESPACE

//! Apply an NOE distance restraint between two particles.
/** The source code is as follows:
    \include NOERestraint.h
    \include NOERestraint.cpp
*/
class IMPISDEXPORT NOEPairScore : public PairScore
{
  Pointer<Particle> sigma_;
  Pointer<Particle> gamma_;
  double Iexp_;
public:
  NOEPairScore(Particle * sigma, Particle * gamma);
  void set_Iexp(double f);
  IMP_PAIR_SCORE(NOEPairScore);
};

IMP_OBJECTS(NOEPairScore, NOEPairScores);


IMPISD_END_NAMESPACE

#endif  /* IMPISD_NOE_PAIR_SCORE_H */
