/**
 *  \file CoverRefinedSingletonModifier.cpp
 *  \brief Cover a the refined particles with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/CoverRefinedSingletonModifier.h"

#include "IMP/core/BondDecoratorListScoreState.h"
#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZRDecorator.h"

IMPCORE_BEGIN_NAMESPACE

CoverRefinedSingletonModifier
::CoverRefinedSingletonModifier(ParticleRefiner *ref,
                                FloatKey rk,
                                  Float slack): ref_(ref),
                                                rk_(rk),
                                                slack_(slack)
{
}

CoverRefinedSingletonModifier::~CoverRefinedSingletonModifier()
{
}

void CoverRefinedSingletonModifier::apply(Particle *p) const
{
  XYZRDecorator dp(p, rk_);
  IMP_check(ref_->get_can_refine(p), "Passed particles cannot be refined",
            ValueException);
  Particles ps= ref_->get_refined(p);
  set_enclosing_sphere(ps, dp);
  dp.set_radius(dp.get_radius()+slack_);
  ref_->cleanup_refined(p, ps);
}

void CoverRefinedSingletonModifier::show(std::ostream &out) const
{
  out << "CoverRefinedSingletonModifier" << std::endl;
}

IMPCORE_END_NAMESPACE
