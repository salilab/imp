/**
 *  \file MyRestraint.h   \brief A restraint on two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMODULENAME_MY_RESTRAINT_H
#define IMPMODULENAME_MY_RESTRAINT_H

#include "config.h"
#include "internal/modulename_version_info.h"
#include <IMP/PairScore.h>
#include <IMP/Restraint.h>

#include <iostream>

IMPMODULENAME_BEGIN_NAMESPACE

//! Restrain a pair of particles
/**
 */
class IMPMODULENAMEEXPORT MyRestraint : public Restraint
{
  Pointer<PairScore> ps_;
public:
  //! Create the restraint.
  MyRestraint(PairScore* score_func,
              Particle* p1, Particle* p2);
  virtual ~MyRestraint() {}

  IMP_RESTRAINT(internal::modulename_version_info)
};

IMPMODULENAME_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_RESTRAINT_H */
