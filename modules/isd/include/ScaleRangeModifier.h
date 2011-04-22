/**
 *  \file isd/ScaleRangeModifier.h
 *  \brief A singleton modifier which wraps an attribute into a
 *  given range.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_SINGLETON_MODIFIER_H
#define IMPISD_SINGLETON_MODIFIER_H

#include "isd_config.h"
#include <IMP/SingletonModifier.h>

IMPISD_BEGIN_NAMESPACE

//! An example singleton modifer
/**  A simple singleton modifier which constrains a Scale particle
 *   to it's allowed domain.

     Use with IMP::core::SingletonRestraint

     \pythonexample{range_restriction}

     The source code is as follows:
     \include ScaleRangeModifier.h
     \include ScaleRangeModifier.cpp
 */
class IMPISDEXPORT ScaleRangeModifier: public SingletonModifier
{
public:
  ScaleRangeModifier() {};

  // note, Doxygen wants a semicolon at the end of macro lines
  IMP_SINGLETON_MODIFIER(ScaleRangeModifier);
};


IMPISD_END_NAMESPACE

#endif  /* IMPISD_SCALE_RANGE_MODIFIER_H */
