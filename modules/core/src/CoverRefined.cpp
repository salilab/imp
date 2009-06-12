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
               Float slack): ref_(ref),
                             rk_(rk),
                             slack_(slack)
{
}

CoverRefined::~CoverRefined()
{
}

void CoverRefined::apply(Particle *p) const
{
  XYZR dp(p, rk_);
  IMP_CHECK_OBJECT(ref_.get());
  IMP_check(ref_->get_can_refine(p), "Passed particles cannot be refined",
            ValueException);
  XYZs ps(ref_->get_refined(p));
  set_enclosing_sphere(dp, ps);
  dp.set_radius(dp.get_radius()+slack_);
}

void CoverRefined::show(std::ostream &out) const
{
  out << "CoverRefined with "
      << *ref_ << " and " << rk_ << std::endl;
}

IMPCORE_END_NAMESPACE
