/**
 *  \file interna/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_CONSTANTS_H
#define IMPKERNEL_INTERNAL_CONSTANTS_H

#include "units.h"

#include <cmath>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Avagadro's number
extern IMPKERNELEXPORT const unit::ExponentialNumber<23> NA;

//! Boltzmann constant in J/K
extern IMPKERNELEXPORT const
unit::Shift<unit::Divide<unit::Joule, unit::Kelvin>::type, -23>::type KB;

//! the default temperature
extern IMPKERNELEXPORT const unit::Kelvin DEFAULT_TEMPERATURE;

extern IMPKERNELEXPORT const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE;

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_CONSTANTS_H */
