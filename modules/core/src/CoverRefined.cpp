/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/core/CoverRefined.h"

#include "IMP/core/XYZR.h"
#include "IMP/core/FixedRefiner.h"
#include "IMP/core/SingletonScoreState.h"
#include "IMP/core/SingletonsScoreState.h"
#include "IMP/core/DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

CoverRefined
::CoverRefined(Refiner *ref,
               FloatKey rk,
               Float slack): rk_(rk),
                             slack_(slack)
{
  refiner_=ref;
}

IMP_SINGLETON_MODIFIER_FROM_REFINED(CoverRefined, refiner_)


void CoverRefined::apply(Particle *p) const
{
  XYZR dp(p, rk_);
  XYZs ps(refiner_->get_refined(p));
  set_enclosing_sphere(dp, ps);
  dp.set_radius(dp.get_radius()+slack_);
}


IMPCORE_END_NAMESPACE
