/**
 *  \file DerivativesFromRefined.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include "IMP/core/DerivativesFromRefined.h"
#include <IMP/refiner_macros.h>
#include "IMP/core/XYZ.h"

IMPCORE_BEGIN_NAMESPACE

DerivativesFromRefined
::DerivativesFromRefined(Refiner *r,
                         FloatKeys ks): refiner_(r),
                                        ks_(ks)
{
}

void DerivativesFromRefined
::apply_index(Model *m,
              ParticleIndex pi) const
{
  Particle *p= m->get_particle(pi);
  DerivativeAccumulator da;
  ParticlesTemp ps = refiner_->get_refined(p);

  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      Float f= ps[i]->get_derivative(ks_[j]);
      p->add_to_derivative(ks_[j], f, da);
    }
  }
}

IMP_SINGLETON_MODIFIER_FROM_REFINED(DerivativesFromRefined, refiner_);

IMPCORE_END_NAMESPACE
