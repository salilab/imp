/**
 *  \file PairScore.cpp  \brief A Score on a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/PairScore.h>

IMP_BEGIN_NAMESPACE

PairScore::~PairScore()
{
  /* Implemented here rather than in the header so that PairScore
     symbols are present in the kernel DSO */
}

IMP_END_NAMESPACE
