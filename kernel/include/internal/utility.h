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

//! Remove all the inactive particles from a vector of particles
inline void remove_inactive_particles(Particles &ps)
{
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          internal::IsInactiveParticle()), ps.end());
  for (Particles::iterator c = ps.begin(); c != ps.end(); ++c) {
    (*c)->assert_is_valid();
    IMP_assert((*c)->get_is_active(), "Did not remove inactive particle");
  }
}

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_INTERNAL_UTILITY_H */
