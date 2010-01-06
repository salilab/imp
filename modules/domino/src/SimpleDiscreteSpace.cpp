/**
 *  \file SimpleDiscreteRestraint.h
 *  \brief Simple restraint for testing
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/domino/SimpleDiscreteSpace.h>
#include <IMP/domino/CombState.h>
#include <IMP/container_macros.h>


IMPDOMINO_BEGIN_NAMESPACE

FloatKey SimpleDiscreteSpace::get_optimization_key() {
  static FloatKey opt_key("OPT");
  return opt_key;
}

SimpleDiscreteSpace::~SimpleDiscreteSpace(){}

IMPDOMINO_END_NAMESPACE
