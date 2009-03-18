/**
 *  \file SingletonScore.cpp  \brief A Score on a single particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/SingletonScore.h>

IMP_BEGIN_NAMESPACE

SingletonScore::SingletonScore()
{
  /* Implemented here rather than in the header so that SingletonScore
     symbols are present in the kernel DSO */
}

IMP_END_NAMESPACE
