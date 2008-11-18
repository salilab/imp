/**
 *  \file CoverBondSingletonModifier.cpp
 *  \brief Cover a bond with a sphere.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/core/CoverBondSingletonModifier.h"

#include "IMP/core/BondDecoratorListScoreState.h"
#include "IMP/core/bond_decorators.h"
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

CoverBondSingletonModifier
::CoverBondSingletonModifier(FloatKey rk,
                            Float slack): rk_(rk), slack_(slack)
{
}

CoverBondSingletonModifier::~CoverBondSingletonModifier()
{
}

void CoverBondSingletonModifier::apply(Particle *p)
{
  BondDecorator bd(p);
  BondedDecorator pa= bd.get_bonded(0);
  BondedDecorator pb= bd.get_bonded(1);
  IMP_LOG(VERBOSE, "Processing bond between "
          << pa.get_particle()->get_index()
          << " and " << pb.get_particle()->get_index() << std::endl);
  XYZDecorator da(pa.get_particle());
  XYZDecorator db(pb.get_particle());
  IMP_LOG(VERBOSE, "Endpoints are " << da << " and "
          << db << std::endl);
  Vector3D diff= da.get_vector_to(db);
  float len= diff.get_magnitude();
  Vector3D cv= da.get_coordinates() + .5*diff;

  XYZRDecorator dxyz(p, rk_);
  dxyz.set_coordinates(cv);

  dxyz.set_radius(len/2.0);

  IMP_LOG(VERBOSE, "Bond cover is " << cv << ": " << len/2.0 << std::endl);
}

void CoverBondSingletonModifier::show(std::ostream &out) const
{
  out << "CoverBondSingletonModifier" << std::endl;
}

IMPCORE_END_NAMESPACE
