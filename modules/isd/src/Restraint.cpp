/**
 *  \file IMP/isd/Restraint.h
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Restraint.h>

IMPISD_BEGIN_NAMESPACE
/** A base class for ISD Restraints.
 */

Restraint::Restraint(IMP::Model *m, std::string name)
    : IMP::Restraint(m, name) {} 

double Restraint::get_probability() const {return 1.0;}

double Restraint::unprotected_evaluate(IMP::DerivativeAccumulator *accum) const {return -log(get_probability());}

IMP::ModelObjectsTemp Restraint::do_get_inputs() const {return IMP::ParticlesTemp();}

IMPISD_END_NAMESPACE
