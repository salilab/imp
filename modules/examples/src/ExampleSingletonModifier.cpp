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

void ExampleSingletonModifier::show(std::ostream &out) const {
  out << "ExampleSingletonModifier" << std::endl;
}

IMPEXAMPLES_END_NAMESPACE
