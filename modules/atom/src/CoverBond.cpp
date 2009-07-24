/**
 *  \file CoverBond.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/atom/CoverBond.h"
#include <IMP/atom/bond_decorators.h>
#include <IMP/core/XYZR.h>

IMPATOM_BEGIN_NAMESPACE

CoverBond::CoverBond( ){
}

void CoverBond::apply(Particle *p) const {
  Bond bd(p);
  core::XYZ ea(bd.get_bonded(0)), eb(bd.get_bonded(1));
  core::XYZR r(p);
  r.set_coordinates(.5*(ea.get_coordinates()+ eb.get_coordinates()));
  r.set_radius((r.get_coordinates()- ea.get_coordinates()).get_magnitude());
}


void CoverBond::show(std::ostream &out) const {
  out << "CoverBond" << std::endl;
}

IMPATOM_END_NAMESPACE
