/**
 *  \file atom/angle_decorators.cpp   \brief Decorators for angles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/angle_decorators.h>

IMPATOM_BEGIN_NAMESPACE

ParticleKey Angle::get_particle_key(unsigned int pi)
{
  static ParticleKey k[3] = {
      ParticleKey("angle particle 1"), ParticleKey("angle particle 2"),
      ParticleKey("angle particle 3") };
  return k[pi];
}

FloatKey Angle::get_ideal_key()
{
  static FloatKey k("ideal");
  return k;
}

FloatKey Angle::get_stiffness_key()
{
  static FloatKey k("stiffness");
  return k;
}

void Angle::show(std::ostream &out) const {
  out << "Angle on " << get_particle(0)->get_name()
      << " " << get_particle(1)->get_name() << " "
      << get_particle(2)->get_name();
}

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

IntKey Dihedral::get_multiplicity_key()
{
  static IntKey k("multiplicity");
  return k;
}

void Dihedral::show(std::ostream &out) const {
  out << "Dihedral on " << get_particle(0)->get_name()
      << " " << get_particle(1)->get_name() << " "
      << get_particle(2)->get_name() << " " << get_particle(3)->get_name();
}

IMPATOM_END_NAMESPACE
