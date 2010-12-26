/**
 *  \file ExamplePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_PAIR_SCORE_H
#define IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_PAIR_SCORE_H

#include "example_system_local_config.h"
#include <IMP/core/XYZ.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPEXAMPLESYSTEMLOCAL_BEGIN_NAMESPACE

//! Apply a harmonic to the distance between two particles.
/** The source code is as follows:
    \include ExampleRestraint.h
    \include ExampleRestraint.cpp
*/
class IMPEXAMPLESYSTEMLOCALEXPORT ExamplePairScore : public PairScore
{
  double x0_, k_;
public:
  ExamplePairScore(double x0, double k);
  IMP_PAIR_SCORE(ExamplePairScore);
};

IMP_OBJECTS(ExamplePairScore, ExamplePairScores);


IMPEXAMPLESYSTEMLOCAL_END_NAMESPACE

#endif  /* IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_PAIR_SCORE_H */
