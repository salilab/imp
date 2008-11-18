/**
 *  \file PropagateBondDerivativesSingletonModifier.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/PropagateBondDerivativesSingletonModifier.h"

#include "IMP/core/BondDecoratorListScoreState.h"
#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

PropagateBondDerivativesSingletonModifier
::PropagateBondDerivativesSingletonModifier()
{
}

PropagateBondDerivativesSingletonModifier
::~PropagateBondDerivativesSingletonModifier()
{
}

void PropagateBondDerivativesSingletonModifier::apply(Particle *p)
{
  DerivativeAccumulator da;
  XYZRDecorator d(p);
  Vector3D deriv= d.get_derivatives();
  deriv/=2.0;

  BondDecorator bd(p);
  for (int i=0; i< 2; ++i) {
    BondedDecorator ep= bd.get_bonded(i);
    XYZDecorator d(ep.get_particle());
    for (int i = 0; i < 3; ++i) {
      d.add_to_coordinate_derivative(i, deriv[i], da);
    }
  }
}

void PropagateBondDerivativesSingletonModifier::show(std::ostream &out) const
{
  out << "PropagateBondDerivativesSingletonModifier" << std::endl;
}

IMPCORE_END_NAMESPACE
