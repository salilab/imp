/**
 *  \file Base_Types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BASE_TYPES_H
#define __IMP_BASE_TYPES_H

#include <string>

#include "IMP_config.h"
#include "Index.h"
#include <iostream>
namespace IMP
{

//! Basic floating-point value (could be float, double...)
typedef float Float;

//! Basic integer value
typedef int Int;

//! Basic string value
typedef std::string String;



struct ParticleTag{};
struct RestraintTag{};
struct StateTag{};

typedef Index<Int> IntIndex;
typedef Index<Float> FloatIndex;
typedef Index<String> StringIndex;
typedef Index<ParticleTag> ParticleIndex;
typedef Index<RestraintTag> RestraintIndex;
typedef Index<StateTag> StateIndex;
// typedefs for the particle variable and attribute indexes
// typedef DataIndex<Float> FloatIndex;
// typedef DataIndex<Int> IntIndex;
// typedef DataIndex<String> StringIndex;

} // namespace IMP

#endif  /* __IMP_BASE_TYPES_H */
