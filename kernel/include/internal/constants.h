/**
 *  \file interna/constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_CONSTANTS_H
#define IMP_INTERNAL_CONSTANTS_H

#include "units.h"

#include <cmath>

IMP_BEGIN_INTERNAL_NAMESPACE

//! Avagadro's number
extern IMPEXPORT const unit::ExponentialNumber<23> NA;

//! Boltzmann constant in J/K
extern IMPEXPORT const
unit::Shift<unit::Divide<unit::Joule, unit::Kelvin>::type, -23>::type KB;

//! the default temperature
extern IMPEXPORT const unit::Kelvin DEFAULT_TEMPERATURE;

extern IMPEXPORT const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE;

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_CONSTANTS_H */
