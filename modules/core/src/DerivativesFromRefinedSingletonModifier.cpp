/**
 *  \file DerivativesFromRefinedSingletonModifier.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/DerivativesFromRefinedSingletonModifier.h"

#include "IMP/core/BondDecoratorListScoreState.h"
#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

DerivativesFromRefinedSingletonModifier
::DerivativesFromRefinedSingletonModifier(ParticleRefiner *r,
                                          FloatKeys ks): r_(r), ks_(ks)
{
}

DerivativesFromRefinedSingletonModifier
::~DerivativesFromRefinedSingletonModifier()
{
}

void DerivativesFromRefinedSingletonModifier::apply(Particle *p) const
{
  Particles ps = r_->get_refined(p);

  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      Float f= ps[i]->get_derivative(ks_[j]);
      DerivativeAccumulator da;
      p->add_to_derivative(ks_[j], f, da);
    }
  }
  r_->cleanup_refined(p, ps);
}

void DerivativesFromRefinedSingletonModifier::show(std::ostream &out) const
{
  out << "DerivativesFromRefinedSingletonModifier" << std::endl;
}

IMPCORE_END_NAMESPACE
