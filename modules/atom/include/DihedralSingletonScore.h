/**
 *  \file atom/DihedralSingletonScore.h
 *  \brief A score on a dihedral angle.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_DIHEDRAL_SINGLETON_SCORE_H
#define IMPATOM_DIHEDRAL_SINGLETON_SCORE_H

#include "config.h"
#include <IMP/SingletonScore.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the dihedral angle.
class IMPATOMEXPORT DihedralSingletonScore : public SingletonScore
{
public:
  DihedralSingletonScore();
  IMP_SINGLETON_SCORE(DihedralSingletonScore);
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_DIHEDRAL_SINGLETON_SCORE_H */
