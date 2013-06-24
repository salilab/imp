/**
 *  \file IMP/atom/angle_decorators.h     \brief Decorators for angles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ANGLE_DECORATORS_H
#define IMPATOM_ANGLE_DECORATORS_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/XYZ.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A particle that describes an angle between three particles.
/** An Angle decorator is a simple container of three particles, together
    with an ideal value (in radians) for the angle and a stiffness.

    \see CHARMMParameters::create_angles(), AngleSingletonScore.
 */
class IMPATOMEXPORT Angle : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex p,
                                core::XYZ a, core::XYZ b,
                                core::XYZ c) {
    m->add_attribute(get_particle_key(0), p, a);
    m->add_attribute(get_particle_key(1), p, b);
    m->add_attribute(get_particle_key(2), p, c);
  }
 public:
  IMP_DECORATOR_METHODS(Angle, Decorator);
  IMP_DECORATOR_SETUP_3(Angle, core::XYZ, a, core::XYZ, b,
                        core::XYZ, c);

  //! Return true if the particle is an angle.
  static bool get_is_setup(Model* m, ParticleIndex pi) {
    for (unsigned int i = 0; i < 3; ++i) {
      if (!m->get_has_attribute(get_particle_key(i), pi)) return false;
    }
    return true;
  }

  Particle* get_particle() const { return Decorator::get_particle(); }

  //! Get the ith particle in the angle.
  Particle* get_particle(unsigned int i) const {
    return get_particle()->get_value(get_particle_key(i));
  }

  IMP_DECORATOR_GET_SET_OPT(ideal, get_ideal_key(), Float, Float, -1);
  IMP_DECORATOR_GET_SET_OPT(stiffness, get_stiffness_key(), Float, Float, 0.);

  static ParticleIndexKey get_particle_key(unsigned int i);
  static FloatKey get_ideal_key();
  static FloatKey get_stiffness_key();
};

IMP_DECORATORS(Angle, Angles, ParticlesTemp);

//! A particle that describes a dihedral angle between four particles.
/** An Angle decorator is a simple container of four particles, together
    with an ideal value (in radians) for the angle, a multiplicity
    and a stiffness.

    Note that multiple Dihedral particles can exist for the same set of
    four particles. (For example, the CHARMM forcefield allows for multiple
    dihedrals to exist with different multiplicities.)

    \see CHARMMParameters::create_dihedrals(),
         CHARMMTopology::add_impropers(), DihedralSingletonScore,
         ImproperSingletonScore.
 */
class IMPATOMEXPORT Dihedral : public Decorator {
  //! Create a dihedral with the given particles.
  static void do_setup_particle(Model *m, ParticleIndex p,
                                 core::XYZ a, core::XYZ b,
                                 core::XYZ c, core::XYZ d) {
    m->add_attribute(get_particle_key(0), p, a);
    m->add_attribute(get_particle_key(1), p, b);
    m->add_attribute(get_particle_key(2), p, c);
    m->add_attribute(get_particle_key(3), p, d);
  }
 public:
  IMP_DECORATOR_METHODS(Dihedral, Decorator);
  IMP_DECORATOR_SETUP_4(Dihedral,
                        core::XYZ, a,
                        core::XYZ, b,
                        core::XYZ, c,
                        core::XYZ, d);

  static bool get_is_setup(Model* m, ParticleIndex pi) {
    for (unsigned int i = 0; i < 4; ++i) {
      if (!m->get_has_attribute(get_particle_key(i), pi)) return false;
    }
    return true;
  }

  Particle* get_particle() const { return Decorator::get_particle(); }

  //! Get the ith particle in the dihedral.
  Particle* get_particle(unsigned int i) const {
    return get_particle()->get_value(get_particle_key(i));
  }

  IMP_DECORATOR_GET_SET_OPT(ideal, get_ideal_key(), Float, Float, -1);
  IMP_DECORATOR_GET_SET_OPT(multiplicity, get_multiplicity_key(), Int, Int, 0);
  IMP_DECORATOR_GET_SET_OPT(stiffness, get_stiffness_key(), Float, Float, 0.);

  static ParticleIndexKey get_particle_key(unsigned int i);
  static FloatKey get_ideal_key();
  static IntKey get_multiplicity_key();
  static FloatKey get_stiffness_key();
};

IMP_DECORATORS(Dihedral, Dihedrals, ParticlesTemp);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ANGLE_DECORATORS_H */
