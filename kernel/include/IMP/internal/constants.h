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

namespace IMP
{

namespace internal
{

// make them doubles so we don't have to worry about digits

//! Avagadro's number
extern IMPDLLEXPORT const ExponentialNumber<23> NA;
//! Boltzmann constant in J/K
extern IMPDLLEXPORT const MKSUnit<-23, 2, 1, -1, -2>  KB;

//! the default temperature
extern IMPDLLEXPORT const Kelvin T;

//! Pi
static const double PI = 3.1415926535897931;

} // namespace internal

} // namespace IMP

#endif  /* __IMP_CONSTANTS_H */
