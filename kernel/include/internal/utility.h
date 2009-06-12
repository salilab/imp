/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_INTERNAL_UTILITY_H
#define IMP_INTERNAL_UTILITY_H

#include "../config.h"

#include <algorithm>

IMP_BEGIN_NAMESPACE

namespace internal
{

//! \internal \return true if a passed particle is inactive
struct IsInactiveParticle
{
  bool operator()(Particle *p) const {
    return !p->get_is_active();
  }
};

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_INTERNAL_UTILITY_H */
