/**
 *  \file RestraintInfo.cpp
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/RestraintInfo.h>
#include <boost/algorithm/string/predicate.hpp>

IMPKERNEL_BEGIN_NAMESPACE

namespace {
bool is_string_key_path(std::string name) {
  return boost::algorithm::ends_with(name, "filename") ||
         boost::algorithm::ends_with(name, "filenames") ||
         name == "cluster density" ||
         name == "image files" ||
         name == "path";
}
}

void RestraintInfo::add_int(std::string key, int value) {
  int_.push_back(IntData(key, value));
}

void RestraintInfo::add_float(std::string key, double value) {
  float_.push_back(FloatData(key, value));
}

void RestraintInfo::add_string(std::string key, std::string value) {
  IMP_USAGE_CHECK(!is_string_key_path(key),
            "Keys used for strings must not end in 'filename' or 'filenames'");
  string_.push_back(StringData(key, value));
}

void RestraintInfo::add_filename(std::string key, std::string value) {
  IMP_USAGE_CHECK(is_string_key_path(key),
            "Keys used for filenames must end in 'filename' or 'filenames'");
  filename_.push_back(StringData(key, value));
}

void RestraintInfo::add_floats(std::string key, Floats value) {
  floats_.push_back(FloatsData(key, value));
}

void RestraintInfo::add_ints(std::string key, Ints value) {
  ints_.push_back(IntsData(key, value));
}

void RestraintInfo::add_strings(std::string key, Strings value) {
  IMP_USAGE_CHECK(!is_string_key_path(key),
            "Keys used for strings must not end in 'filename' or 'filenames'");
  strings_.push_back(StringsData(key, value));
}

void RestraintInfo::add_filenames(std::string key, Strings value) {
  IMP_USAGE_CHECK(is_string_key_path(key),
            "Keys used for filenames must end in 'filename' or 'filenames'");
  filenames_.push_back(StringsData(key, value));
}

void RestraintInfo::add_particle_indexes(
    std::string key, ParticleIndexes value)
{
  pis_.push_back(ParticleIndexesData(key, value));
}

IMPKERNEL_END_NAMESPACE
