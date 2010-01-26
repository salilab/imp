/**
 *  \file atom/LennardJones.h
 *  \brief A decorator for a particle that has a Lennard-Jones potential well.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_LENNARD_JONES_H
#define IMPATOM_LENNARD_JONES_H

#include "config.h"
#include <IMP/core/internal/utility.h>
#include <IMP/core/XYZR.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a particle that has a Lennard-Jones potential well.
/** Such particles must be XYZR particles (they must have a position and
    a radius) but need not be true atoms.
    \ingroup helper
    \ingroup decorators
    \see LennardJonesPairScore
 */
class IMPATOMEXPORT LennardJones: public IMP::core::XYZR
{
public:
  IMP_DECORATOR(LennardJones, IMP::core::XYZR)

  /** Create a decorator with the passed well depth.
      The particle is assumed to already have x,y,z,r attributes.
   */
  static LennardJones setup_particle(Particle *p, Float well_depth=0) {
    IMP_USAGE_CHECK(XYZR::particle_is_instance(p),
                    "Particle must already be an XYZR particle",
                    ValueException);
    p->add_attribute(get_well_depth_key(), well_depth);
    return LennardJones(p);
  }

  IMP_DECORATOR_GET_SET(well_depth, get_well_depth_key(), Float, Float);

  //! Return true if the particle is an instance of a LennardJones
  static bool particle_is_instance(Particle *p) {
    return XYZR::particle_is_instance(p)
           && p->has_attribute(get_well_depth_key());
  }

  static FloatKey get_well_depth_key();
};

IMP_OUTPUT_OPERATOR(LennardJones);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_LENNARD_JONES_H */
