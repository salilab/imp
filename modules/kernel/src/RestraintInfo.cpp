/**
 *  \file RestraintInfo.cpp
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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

void RestraintInfo::add_string(std::string key, std::string value) {
  string_.push_back(StringData(key, value));
}

void RestraintInfo::add_filename(std::string key, std::string value) {
  filename_.push_back(StringData(key, value));
}

void RestraintInfo::add_floats(std::string key, Floats value) {
  floats_.push_back(FloatsData(key, value));
}

void RestraintInfo::add_ints(std::string key, Ints value) {
  ints_.push_back(IntsData(key, value));
}

void RestraintInfo::add_strings(std::string key, Strings value) {
  strings_.push_back(StringsData(key, value));
}

void RestraintInfo::add_filenames(std::string key, Strings value) {
  filenames_.push_back(StringsData(key, value));
}

void RestraintInfo::add_particle_indexes(
    std::string key, ParticleIndexes value)
{
  pis_.push_back(ParticleIndexesData(key, value));
}

IMPKERNEL_END_NAMESPACE
