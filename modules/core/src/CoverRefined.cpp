/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/core/CoverRefined.h"

#include "IMP/core/XYZR.h"
#include "IMP/core/FixedRefiner.h"
#include "IMP/core/SingletonConstraint.h"
#include "IMP/core/SingletonsConstraint.h"
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
  set_enclosing_sphere(dp, ps, slack_);
}


IMP_SUMMARY_DECORATOR_DEF(Cover, XYZR, XYZs,
                          SingletonModifier * mod
                          = new CoverRefined(ref,
                                             XYZR::get_default_radius_key(), 0))
IMPCORE_END_NAMESPACE
