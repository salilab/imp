/**
 *  \file RestraintInfo.cpp
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/RestraintInfo.h>

IMPKERNEL_BEGIN_NAMESPACE

void RestraintInfo::add_floats(std::string key, Floats value) {
  floats_.push_back(FloatsData(key, value));
}

IMPKERNEL_END_NAMESPACE
