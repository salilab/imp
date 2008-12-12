/**
 *  \file constants.h    \brief Various useful constants.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_CONSTANTS_H
#define IMP_CONSTANTS_H

#include "units.h"

#include <cmath>

IMP_BEGIN_NAMESPACE

namespace internal
{

// make them doubles so we don't have to worry about digits

//! Avagadro's number
extern IMPEXPORT const unit::ExponentialNumber<23> NA;

//! Boltzmann constant in J/K
extern IMPEXPORT const
unit::Shift<unit::Divide<unit::Joule, unit::Kelvin>::type, -23>::type KB;

//! the default temperature
extern IMPEXPORT const unit::Kelvin DEFAULT_TEMPERATURE;

//! Pi
static const double PI = 3.1415926535897931;

extern IMPEXPORT const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE;

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_CONSTANTS_H */
