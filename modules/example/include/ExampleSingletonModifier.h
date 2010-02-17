/**
 *  \file example/ExampleSingletonModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H
#define IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H

#include "config.h"
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example singleton modifer
/**  A simple singleton modifier which restrains the x,y,z
     coordinates to a box by wrapping them.

     Such a class could be coupled with an IMP::core::SingletonRestraint
     or IMP::core::SingletonsRestraint to keep a set of particles
     in a box.

     \htmlinclude range_restriction.py.html

     The source code is as follows:
     \include ExampleRestraint.h
     \include ExampleRestraint.cpp
 */
class IMPEXAMPLEEXPORT ExampleSingletonModifier: public SingletonModifier
{
  algebra::BoundingBox3D bb_;
public:
  ExampleSingletonModifier(const algebra::BoundingBox3D &bb);

  IMP_SINGLETON_MODIFIER(ExampleSingletonModifier)
};


IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H */
