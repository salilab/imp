/**
 *  \file ExampleSingletonModifier.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/examples/ExampleSingletonModifier.h"
#include "IMP/core/XYZ.h"


IMPEXAMPLES_BEGIN_NAMESPACE
ExampleSingletonModifier
::ExampleSingletonModifier(const algebra::BoundingBox3D &bb ): bb_(bb){
}

void ExampleSingletonModifier::apply(Particle *p) const {
  core::XYZ d(p);
  for (unsigned int i=0; i< 3; ++i) {
    // shift the coordinate until it is in the box
    while (d.get_coordinate(i) < bb_.get_corner(0)[i]) {
      d.set_coordinate(i, d.get_coordinate(i)
                       + (bb_.get_corner(1)[i]-bb_.get_corner(0)[i]));
    }
    while (d.get_coordinate(i) > bb_.get_corner(1)[i]) {
      d.set_coordinate(i, d.get_coordinate(i)
                       - (bb_.get_corner(1)[i]-bb_.get_corner(0)[i]));
    }
  }
}

/* There are no interactions created by this modifier */
ParticlesList
ExampleSingletonModifier::get_interacting_particles(Particle *) const {
  return ParticlesList();
}

/* Only the passed particle is used */
ParticlesTemp
ExampleSingletonModifier::get_read_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ParticlesTemp
ExampleSingletonModifier::get_write_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

void ExampleSingletonModifier::show(std::ostream &out) const {
  out << "ExampleSingletonModifier" << std::endl;
}

IMPEXAMPLES_END_NAMESPACE
