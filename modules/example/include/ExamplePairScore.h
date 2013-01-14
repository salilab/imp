/**
 *  \file IMP/example/ExamplePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_PAIR_SCORE_H
#define IMPEXAMPLE_EXAMPLE_PAIR_SCORE_H

#include <IMP/example/example_config.h>
#include <IMP/core/XYZ.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! Apply a harmonic to the distance between two particles.
/** The source code is as follows:
    \include ExamplePairScore.h
    \include ExamplePairScore.cpp
*/
class IMPEXAMPLEEXPORT ExamplePairScore : public PairScore
{
  double x0_, k_;
public:
  ExamplePairScore(double x0, double k);
  IMP_PAIR_SCORE(ExamplePairScore);
};

IMP_OBJECTS(ExamplePairScore, ExamplePairScores);


IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_PAIR_SCORE_H */
