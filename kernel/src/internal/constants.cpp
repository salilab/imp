/**
 *  \file constants.cpp  \brief Various useful constants.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/internal/constants.h"

IMP_BEGIN_NAMESPACE

namespace internal
{

const unit::ExponentialNumber<23> NA(6.02214179);

const unit::internal::AtomsPerMol unit::ATOMS_PER_MOL = {};

// definition of KB
const unit::Shift<unit::Divide<unit::Joule, unit::Kelvin>::type, -23>::type
KB(1.3806503);

const unit::Kelvin DEFAULT_TEMPERATURE(297.15);


const unit::ExponentialNumber<3> JOULES_PER_KILOCALORIE(4.1868);

} // namespace internal

IMP_END_NAMESPACE
