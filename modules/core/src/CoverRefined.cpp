/**
 *  \file CoverRefined.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include "IMP/core/CoverRefined.h"

#include "IMP/core/XYZR.h"
#include "IMP/core/FixedRefiner.h"
#include "IMP/core/SingletonConstraint.h"
#include "IMP/core/DerivativesToRefined.h"

IMPCORE_BEGIN_NAMESPACE

CoverRefined
::CoverRefined(Refiner *ref,
               Float slack): slack_(slack)
{
  refiner_=ref;
}

IMP_SINGLETON_MODIFIER_FROM_REFINED(CoverRefined, refiner_);


void CoverRefined::apply(Particle *p) const
{
  XYZR dp(p);
  XYZsTemp ps(refiner_->get_refined(p));
  set_enclosing_sphere(dp, ps, slack_);
}


IMP_SUMMARY_DECORATOR_DEF(Cover, XYZR, XYZs,
                          SingletonModifier * mod
                          = new CoverRefined(ref,0));
IMPCORE_END_NAMESPACE
