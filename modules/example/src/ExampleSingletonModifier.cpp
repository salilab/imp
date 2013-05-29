/**
 *  \file example/ExampleSingletonModifier.cpp
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleSingletonModifier.h"
#include "IMP/core/XYZ.h"

IMPEXAMPLE_BEGIN_NAMESPACE
ExampleSingletonModifier::ExampleSingletonModifier(
    const algebra::BoundingBox3D &bb)
    : bb_(bb) {}

void ExampleSingletonModifier::apply(Particle *p) const {
  core::XYZ d(p);
  for (unsigned int i = 0; i < 3; ++i) {
    // shift the coordinate until it is in the box
    while (d.get_coordinate(i) < bb_.get_corner(0)[i]) {
      d.set_coordinate(i, d.get_coordinate(i) +
                              (bb_.get_corner(1)[i] - bb_.get_corner(0)[i]));
    }
    while (d.get_coordinate(i) > bb_.get_corner(1)[i]) {
      d.set_coordinate(i, d.get_coordinate(i) -
                              (bb_.get_corner(1)[i] - bb_.get_corner(0)[i]));
    }
  }
}

/* Only the passed particle is used */
ParticlesTemp ExampleSingletonModifier::get_input_particles(Particle *p) const {
  return ParticlesTemp(1, p);
}

ParticlesTemp ExampleSingletonModifier::get_output_particles(
    Particle *p) const {
  return ParticlesTemp(1, p);
}

ContainersTemp ExampleSingletonModifier::get_input_containers(
    Particle *) const {
  return ContainersTemp();
}

ContainersTemp ExampleSingletonModifier::get_output_containers(
    Particle *) const {
  return ContainersTemp();
}

void ExampleSingletonModifier::do_show(std::ostream &out) const {
  out << "bounding box " << bb_ << std::endl;
}

IMPEXAMPLE_END_NAMESPACE
