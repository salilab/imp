/**
 *  \file IMP/isd/ISDRestraint.cpp
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/ISDRestraint.h>

IMPISD_BEGIN_NAMESPACE

ISDRestraint::ISDRestraint(IMP::Model *m, std::string name)
    : Restraint(m, name) {}

double ISDRestraint::get_probability() const { return 1.0; }

double ISDRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  return -log(get_probability());
}

ModelObjectsTemp ISDRestraint::do_get_inputs() const {
  return IMP::ModelObjectsTemp();
}

IMPISD_END_NAMESPACE
