/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/internal/close_pairs_helpers.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/utility.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder():
  ClosePairsFinder("QuadraticCPF"){}



void QuadraticClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}




IMPCORE_END_NAMESPACE
