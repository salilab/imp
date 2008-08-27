/**
 *  \file CoverBondsScoreState.cpp
 *  \brief Covers a set of bonds with spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/CoverBondsScoreState.h"
#include "IMP/score_states/BondDecoratorListScoreState.h"
#include "IMP/decorators/bond_decorators.h"
#include "IMP/decorators/XYZDecorator.h"

namespace IMP
{


CoverBondsScoreState::CoverBondsScoreState(BondDecoratorListScoreState *bl,
                                           FloatKey rk): bl_(bl), rk_(rk)
{
}

CoverBondsScoreState::~CoverBondsScoreState()
{
}

void CoverBondsScoreState::do_before_evaluate()
{
  for (BondDecoratorListScoreState::BondIterator it= bl_->bonds_begin();
       it != bl_->bonds_end(); ++it) {
    BondDecorator bd= *it;
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
    Vector3D cv= da.get_vector() + .5*diff;

    XYZDecorator dxyz;
    // slightly evil
    try {
      dxyz= XYZDecorator::cast(bd.get_particle());
    } catch (const InvalidStateException &ve) {
      dxyz= XYZDecorator::create(bd.get_particle());
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
    for (BondDecoratorListScoreState::BondIterator it= bl_->bonds_begin();
         it != bl_->bonds_end(); ++it) {
      XYZDecorator d(it->get_particle());
      Vector3D deriv;
      // divide derivatives equally between endpoints
      for (int i = 0; i < 3; ++i) {
        deriv[i] = d.get_coordinate_derivative(i) / 2.0;
      }

      for (int i=0; i< 2; ++i) {
        BondedDecorator bd= it->get_bonded(i);
        XYZDecorator d(bd.get_particle());
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

} // namespace IMP
