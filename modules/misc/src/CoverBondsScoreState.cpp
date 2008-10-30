/**
 *  \file CoverBondsScoreState.cpp
 *  \brief Covers a set of bonds with spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/misc/CoverBondsScoreState.h>

#include <IMP/core/BondDecoratorListScoreState.h>
#include <IMP/core/bond_decorators.h>
#include <IMP/core/XYZDecorator.h>

IMPMISC_BEGIN_NAMESPACE

CoverBondsScoreState
::CoverBondsScoreState(core::BondDecoratorListScoreState *bl,
                       FloatKey rk): bl_(bl), rk_(rk)
{
}

CoverBondsScoreState::~CoverBondsScoreState()
{
}

void CoverBondsScoreState::do_before_evaluate()
{
  for (core::BondDecoratorListScoreState::BondIterator it= bl_->bonds_begin();
       it != bl_->bonds_end(); ++it) {
    core::BondDecorator bd= *it;
    core::BondedDecorator pa= bd.get_bonded(0);
    core::BondedDecorator pb= bd.get_bonded(1);
    IMP_LOG(VERBOSE, "Processing bond between "
            << pa.get_particle()->get_index()
            << " and " << pb.get_particle()->get_index() << std::endl);
    core::XYZDecorator da(pa.get_particle());
    core::XYZDecorator db(pb.get_particle());
    IMP_LOG(VERBOSE, "Endpoints are " << da << " and "
            << db << std::endl);
    Vector3D diff= da.get_vector_to(db);
    float len= diff.get_magnitude();
    Vector3D cv= da.get_coordinates() + .5*diff;

    core::XYZDecorator dxyz;
    // slightly evil
    try {
      dxyz= core::XYZDecorator::cast(bd.get_particle());
    } catch (const InvalidStateException &ve) {
      dxyz= core::XYZDecorator::create(bd.get_particle());
    }
    dxyz.set_coordinates(cv);

    if (dxyz.get_particle()->has_attribute(rk_)) {
      dxyz.get_particle()->set_value(rk_, len/2.0);
    } else {
      dxyz.get_particle()->add_attribute(rk_, len/2.0, false);
    }
    IMP_LOG(VERBOSE, "Bond cover is " << cv << ": " << len/2.0 << std::endl);
  }
}

void CoverBondsScoreState::after_evaluate(DerivativeAccumulator *dva)
{
  if (dva) {
    for (core::BondDecoratorListScoreState::BondIterator it= bl_->bonds_begin();
         it != bl_->bonds_end(); ++it) {
      core::XYZDecorator d(it->get_particle());
      Vector3D deriv;
      // divide derivatives equally between endpoints
      for (int i = 0; i < 3; ++i) {
        deriv[i] = d.get_coordinate_derivative(i) / 2.0;
      }

      for (int i=0; i< 2; ++i) {
        core::BondedDecorator bd= it->get_bonded(i);
        core::XYZDecorator d(bd.get_particle());
        for (int i = 0; i < 3; ++i) {
          d.add_to_coordinate_derivative(i, deriv[i], *dva);
        }
      }
    }
  }
}

void CoverBondsScoreState::show(std::ostream &out) const
{
  out << "CoverBondsScoreState on " << bl_->get_number_of_bonds()
      << " bonds " << std::endl;
}

IMPMISC_END_NAMESPACE
