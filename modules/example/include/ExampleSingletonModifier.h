/**
 *  \file IMP/example/ExampleSingletonModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H
#define IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H

#include <IMP/example/example_config.h>
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/singleton_macros.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! An example singleton modifer
/**  A simple singleton modifier which restrains the x,y,z
     coordinates to a box by wrapping them.

     Such a class could be coupled with an IMP::core::SingletonRestraint
     or IMP::core::SingletonsRestraint to keep a set of particles
     in a box.

     \include range_restriction.py

     The source code is as follows:
     \include ExampleRestraint.h
     \include ExampleRestraint.cpp
 */
class IMPEXAMPLEEXPORT ExampleSingletonModifier : public SingletonModifier {
  algebra::BoundingBoxD<3> bb_;

 public:
  ExampleSingletonModifier(const algebra::BoundingBoxD<3> &bb);

  // note, Doxygen wants a semicolon at the end of macro lines
  IMP_SINGLETON_MODIFIER(ExampleSingletonModifier);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_SINGLETON_MODIFIER_H */
