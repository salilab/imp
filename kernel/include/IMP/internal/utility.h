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

//! Remove all particles from a range in a container
inline void remove_inactive_particles(Particles &ps,
                                      Particles::iterator b,
                                      Particles::iterator e)
{
  ps.erase(std::remove_if(b, e, internal::IsInactiveParticle()), e);
  for (Particles::iterator c= b; c!= e; ++c) {
    (*c)->assert_is_valid();
    IMP_assert((*c)->get_is_active(), "Did not remove inactive particle");
  }
}

//! Remove all the inactive particles from a vector of particles
inline void remove_inactive_particles(Particles &ps)
{
  remove_inactive_particles(ps, ps.begin(), ps.end());
}

} // namespace internal

} // namespace IMP

#endif  /* __IMP_INTERNAL_UTILITY_H */
