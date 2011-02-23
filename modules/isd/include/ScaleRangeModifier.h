/**
 *  \file isd/ScaleRangeModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_ISD_SINGLETON_MODIFIER_H
#define IMPISD_ISD_SINGLETON_MODIFIER_H

#include "isd_config.h"
#include <IMP/SingletonModifier.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPISD_BEGIN_NAMESPACE

//! An example singleton modifer
/**  A simple singleton modifier which restrains the only     
 *   coordinate of a given 1D particle to a box by wrapping them.

     Such a class could be coupled with an IMP::core::SingletonRestraint
     or IMP::core::SingletonsRestraint to keep a set of particles
     in a box.

     \pythonexample{range_restriction}

     The source code is as follows:
     \include ExampleRestraint.h
     \include ExampleRestraint.cpp
 */
class IMPISDEXPORT ScaleRangeModifier: public SingletonModifier
{
  algebra::BoundingBoxD<1> bb_;
public:
  ScaleRangeModifier(const algebra::BoundingBoxD<1> &bb);

  // note, Doxygen wants a semicolon at the end of macro lines
  IMP_SINGLETON_MODIFIER(ScaleRangeModifier);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_SCALE_RANGE_MODIFIER_H */
