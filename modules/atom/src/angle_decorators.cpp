/**
 *  \file atom/angle_decorators.cpp   \brief Decorators for angles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/angle_decorators.h>

IMPATOM_BEGIN_NAMESPACE

ParticleKey Dihedral::get_particle_key(unsigned int pi)
{
  static ParticleKey k[4] = {
      ParticleKey("dihedral particle 1"), ParticleKey("dihedral particle 2"),
      ParticleKey("dihedral particle 3"), ParticleKey("dihedral particle 4") };
  return k[pi];
}

FloatKey Dihedral::get_ideal_key()
{
  static FloatKey k("ideal");
  return k;
}

FloatKey Dihedral::get_stiffness_key()
{
  static FloatKey k("stiffness");
  return k;
}

FloatKey Dihedral::get_multiplicity_key()
{
  static FloatKey k("multiplicity");
  return k;
}

Dihedral dihedral(Particle *a, Particle *b, Particle *c, Particle *d) {
  return Dihedral::setup_particle(new Particle(a->get_model()), a, b, c, d);
}

IMPATOM_END_NAMESPACE
