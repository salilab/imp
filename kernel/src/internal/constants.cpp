/**
 *  \file constants.cpp  \brief Various useful constants.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/internal/constants.h"

namespace IMP
{

namespace internal
{

const ExponentialNumber<23> NA(6.02214179);

// definition of KB
const MKSUnit<-23, 2, 1, -1, -2>  KB(1.3806503);

const Kelvin T(297.15);


const ExponentialNumber<3> JoulesPerKiloCalorie(4.1868);
const ExponentialNumber<-4> KiloCaloriesPerJoule(2.388459);

} // namespace internal

} // namespace IMP
