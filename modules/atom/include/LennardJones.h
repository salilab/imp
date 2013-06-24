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
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Float well_depth = 0) {
    IMP_USAGE_CHECK(XYZR::get_is_setup(m, pi),
                    "Particle must already be an XYZR particle");
    m->add_attribute(get_well_depth_key(), pi, well_depth);
  }
 public:
  IMP_DECORATOR_METHODS(LennardJones, IMP::core::XYZR);
  IMP_DECORATOR_SETUP_0(LennardJones);
  IMP_DECORATOR_SETUP_1(LennardJones, Float, well_depth);

  Float get_well_depth() const {
    return static_cast<Float>(get_particle()->get_value(get_well_depth_key()));
  }

  void set_well_depth(Float well_depth) {
    IMP_USAGE_CHECK(well_depth >= 0, "well depth cannot be negative");
    get_particle()->set_value(get_well_depth_key(), well_depth);
  }

  //! Return true if the particle is an instance of a LennardJones
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return XYZR::get_is_setup(m, pi) &&
      m->get_has_attribute(get_well_depth_key(), pi);
  }

  static FloatKey get_well_depth_key();
};

IMP_DECORATORS(LennardJones, LennardJonesList, core::XYZRs);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_H */
