/**
 *  \file DerivativesFromRefined.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include "IMP/core/DerivativesFromRefined.h"

#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

DerivativesFromRefined
::DerivativesFromRefined(Refiner *r,
                                          FloatKeys ks): r_(r), ks_(ks)
{
}

DerivativesFromRefined
::~DerivativesFromRefined()
{
}

void DerivativesFromRefined
::apply(Particle *p,
        DerivativeAccumulator *da) const
{
  if (!da) return;
  Particles ps = r_->get_refined(p);

  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      Float f= ps[i]->get_derivative(ks_[j]);
      p->add_to_derivative(ks_[j], f, *da);
    }
  }
}

void DerivativesFromRefined::show(std::ostream &out) const
{
  out << "DerivativesFromRefined" << std::endl;
}

IMPCORE_END_NAMESPACE
