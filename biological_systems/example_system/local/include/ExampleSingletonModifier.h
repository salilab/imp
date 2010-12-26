/**
 *  \file local/ExampleSingletonModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 */

#ifndef IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_SINGLETON_MODIFIER_H
#define IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_SINGLETON_MODIFIER_H

#include "example_system_local_config.h"
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPEXAMPLESYSTEMLOCAL_BEGIN_NAMESPACE

//! An local singleton modifer
/**  A simple singleton modifier which restrains the x,y,z
     coordinates to a box by wrapping them.

     Such a class could be coupled with an IMP::core::SingletonRestraint
     or IMP::core::SingletonsRestraint to keep a set of particles
     in a box.

     \pythonlocal{range_restriction}

     The source code is as follows:
     \include ExampleRestraint.h
     \include ExampleRestraint.cpp
 */
class IMPEXAMPLESYSTEMLOCALEXPORT ExampleSingletonModifier:
  public SingletonModifier
{
  algebra::BoundingBoxD<3> bb_;
public:
  ExampleSingletonModifier(const algebra::BoundingBoxD<3> &bb);

  // note, Doxygen wants a semicolon at the end of macro lines
  IMP_SINGLETON_MODIFIER(ExampleSingletonModifier);
};


IMPEXAMPLESYSTEMLOCAL_END_NAMESPACE

#endif  /* IMP_EXAMPLE_SYSTEM_LOCAL_EXAMPLE_SINGLETON_MODIFIER_H */
