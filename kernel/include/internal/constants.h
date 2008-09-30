/**
 *  \file constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONSTANTS_H
#define __IMP_CONSTANTS_H

#include "units.h"

#include <cmath>

IMP_BEGIN_NAMESPACE

namespace internal
{

// make them doubles so we don't have to worry about digits

//! Avagadro's number
extern IMPDLLEXPORT const unit::ExponentialNumber<23> NA;

//! Boltzmann constant in J/K
extern IMPDLLEXPORT const
unit::Shift<unit::Divide<unit::Joule, unit::Kelvin>::type, -23>::type KB;

//! the default temperature
extern IMPDLLEXPORT const unit::Kelvin DEFAULT_TEMPERATURE;

//! Pi
static const double PI = 3.1415926535897931;

extern IMPDLLEXPORT const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE;

} // namespace internal

IMP_END_NAMESPACE

#endif  /* __IMP_CONSTANTS_H */
