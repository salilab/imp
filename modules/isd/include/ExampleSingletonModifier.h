/**
 *  \file isd/ExampleSingletonModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-8 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_ISD_SINGLETON_MODIFIER_H
#define IMPISD_ISD_SINGLETON_MODIFIER_H

#include "isd_config.h"
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPISD_BEGIN_NAMESPACE

//! An isd singleton modifer
/**  A simple singleton modifier which restrains the x,y,z
     coordinates to a box by wrapping them.

     Such a class could be coupled with an IMP::core::SingletonRestraint
     or IMP::core::SingletonsRestraint to keep a set of particles
     in a box.

     \pythonisd{range_restriction}

     The source code is as follows:
     \include ExampleRestraint.h
     \include ExampleRestraint.cpp
 */
class IMPISDEXPORT ExampleSingletonModifier: public SingletonModifier
{
  algebra::BoundingBoxD<3> bb_;
public:
  ExampleSingletonModifier(const algebra::BoundingBoxD<3> &bb);

  // note, Doxygen wants a semicolon at the end of macro lines
  IMP_SINGLETON_MODIFIER(ExampleSingletonModifier);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_SINGLETON_MODIFIER_H */
