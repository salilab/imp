/**
 *  \file IMP/atom/LennardJones.h
 *  \brief A decorator for a particle that has a Lennard-Jones potential well.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_LENNARD_JONES_H
#define IMPATOM_LENNARD_JONES_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/XYZR.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a particle that has a Lennard-Jones potential well.
/** Such particles must be XYZR particles (they must have a position and
    a radius) but need not be true atoms.

    The well depth should be a non-negative value.
    \ingroup helper
    \ingroup decorators
    \see LennardJonesPairScore
 */
class IMPATOMEXPORT LennardJones : public IMP::core::XYZR {
 public:
  IMP_DECORATOR(LennardJones, IMP::core::XYZR);

  /** Create a decorator with the passed well depth.
      The particle is assumed to already have x,y,z,r attributes.
   */
  static LennardJones setup_particle(Particle *p, Float well_depth = 0) {
    IMP_USAGE_CHECK(XYZR::particle_is_instance(p),
                    "Particle must already be an XYZR particle");
    p->add_attribute(get_well_depth_key(), well_depth);
    return LennardJones(p);
  }

  Float get_well_depth() const {
    return static_cast<Float>(get_particle()->get_value(get_well_depth_key()));
  }

  void set_well_depth(Float well_depth) {
    IMP_USAGE_CHECK(well_depth >= 0, "well depth cannot be negative");
    get_particle()->set_value(get_well_depth_key(), well_depth);
  }

  //! Return true if the particle is an instance of a LennardJones
  static bool particle_is_instance(Particle *p) {
    return XYZR::particle_is_instance(p) &&
           p->has_attribute(get_well_depth_key());
  }

  static FloatKey get_well_depth_key();
};

IMP_DECORATORS(LennardJones, LennardJonesList, core::XYZRs);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_H */
