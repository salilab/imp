/**
 *  \file RestraintInfo.cpp
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/RestraintInfo.h>

IMPKERNEL_BEGIN_NAMESPACE

void RestraintInfo::add_int(std::string key, int value) {
  int_.push_back(IntData(key, value));
}

void RestraintInfo::add_float(std::string key, double value) {
  float_.push_back(FloatData(key, value));
}

void RestraintInfo::add_floats(std::string key, Floats value) {
  floats_.push_back(FloatsData(key, value));
}

void RestraintInfo::add_filenames(std::string key, Strings value) {
  filenames_.push_back(StringsData(key, value));
}

IMPKERNEL_END_NAMESPACE
