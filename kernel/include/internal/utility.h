/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_INTERNAL_UTILITY_H
#define __IMP_INTERNAL_UTILITY_H

#include "../IMP_config.h"

#include <algorithm>

namespace IMP
{

namespace internal
{

//! return true if a passed particle is inactive
struct IMPDLLEXPORT IsInactiveParticle
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

} // namespace IMP

#endif  /* __IMP_INTERNAL_UTILITY_H */
