/**
 *  \file IMP/RestraintInfo.h
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_INFO_H
#define IMPKERNEL_RESTRAINT_INFO_H

#include <IMP/kernel_config.h>
#include <IMP/Object.h>
#include <IMP/types.h>
#include <IMP/base_types.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Report key:value information on restraints
/** These objects are typically returned by Restraint::get_static_info()
    or Restraint::get_dynamic_info() and are used to report information
    about a Restraint instance as a set of key:value pairs. The primary
    purpose is to allow restraints to be written to files, such as RMF.

    Values can be simple types (int, float, string) or lists of them;
    filename(s) (treated similarly to strings but paths are made relative
    to that of the output file); or particles.

    Particle values are used to reference particles that contain restraint
    information (either static or dynamic) that generally exist
    outside of the molecular hierarchy, such as Bayesian nuisances
    or Gaussians for an EM density map. The particles must live in
    the same model as the restraint.
  */
class IMPKERNELEXPORT RestraintInfo : public Object {
public:
  RestraintInfo(std::string name = "RestraintInfo %1%") : Object(name) {}

  //! Add an int value referenced by the given key
  void add_int(std::string key, int value);

  //! Get the number of int that have been added
  unsigned get_number_of_int() const { return int_.size(); }

  //! Get the key for the ith int mapping
  std::string get_int_key(unsigned i) const { return int_[i].first; }

  //! Get the value for the ith int mapping
  int get_int_value(unsigned i) const {
    return int_[i].second;
  }

  //! Add a float value referenced by the given key
  void add_float(std::string key, double value);

  //! Get the number of float that have been added
  unsigned get_number_of_float() const { return float_.size(); }

  //! Get the key for the ith float mapping
  std::string get_float_key(unsigned i) const { return float_[i].first; }

  //! Get the value for the ith float mapping
  double get_float_value(unsigned i) const {
    return float_[i].second;
  }

  //! Add a string value referenced by the given key
  void add_string(std::string key, std::string value);

  //! Get the number of string that have been added
  unsigned get_number_of_string() const { return string_.size(); }

  //! Get the key for the ith string mapping
  std::string get_string_key(unsigned i) const { return string_[i].first; }

  //! Get the value for the ith string mapping
  std::string get_string_value(unsigned i) const {
    return string_[i].second;
  }

  //! Add a filename value referenced by the given key
  /** Filenames are treated similarly to strings but the caller is expected
      to make them absolute paths before adding them here. When written to
      file they may be converted to paths relative to the file.
   */
  void add_filename(std::string key, std::string value);

  //! Get the number of filename that have been added
  unsigned get_number_of_filename() const { return filename_.size(); }

  //! Get the key for the ith filename mapping
  std::string get_filename_key(unsigned i) const { return filename_[i].first; }

  //! Get the value for the ith filename mapping
  std::string get_filename_value(unsigned i) const {
    return filename_[i].second;
  }

  //! Add a list of Float values referenced by the given key
  void add_floats(std::string key, Floats value);

  //! Get the number of Floats that have been added
  unsigned get_number_of_floats() const { return floats_.size(); }

  //! Get the key for the ith Floats mapping
  std::string get_floats_key(unsigned i) const { return floats_[i].first; }

  //! Get the value for the ith Floats mapping
  Floats get_floats_value(unsigned i) const {
    return floats_[i].second;
  }

  //! Add a list of Int values referenced by the given key
  void add_ints(std::string key, Ints value);

  //! Get the number of Ints that have been added
  unsigned get_number_of_ints() const { return ints_.size(); }

  //! Get the key for the ith Ints mapping
  std::string get_ints_key(unsigned i) const { return ints_[i].first; }

  //! Get the value for the ith Ints mapping
  Ints get_ints_value(unsigned i) const {
    return ints_[i].second;
  }

  //! Add a list of string values referenced by the given key
  void add_strings(std::string key, Strings value);

  //! Get the number of strings that have been added
  unsigned get_number_of_strings() const { return strings_.size(); }

  //! Get the key for the ith strings mapping
  std::string get_strings_key(unsigned i) const {
    return strings_[i].first;
  }

  //! Get the value for the ith strings mapping
  Strings get_strings_value(unsigned i) const {
    return strings_[i].second;
  }

  //! Add a list of filename values referenced by the given key
  /** Filenames are treated similarly to strings but the caller is expected
      to make them absolute paths before adding them here. When written to
      file they may be converted to paths relative to the file.
   */
  void add_filenames(std::string key, Strings value);

  //! Get the number of filenames that have been added
  unsigned get_number_of_filenames() const { return filenames_.size(); }

  //! Get the key for the ith filenames mapping
  std::string get_filenames_key(unsigned i) const {
    return filenames_[i].first;
  }

  //! Get the value for the ith filenames mapping
  Strings get_filenames_value(unsigned i) const {
    return filenames_[i].second;
  }

  //! Add ParticleIndexes referenced by the given key
  void add_particle_indexes(std::string key, ParticleIndexes value);

  //! Get the number of ParticleIndexes that have been added
  unsigned get_number_of_particle_indexes() const { return pis_.size(); }

  //! Get the key for the ith ParticleIndexes mapping
  std::string get_particle_indexes_key(unsigned i) const {
    return pis_[i].first;
  }

  //! Get the value for the ith ParticleIndexes mapping
  ParticleIndexes get_particle_indexes_value(unsigned i) const {
    return pis_[i].second;
  }

  IMP_OBJECT_METHODS(RestraintInfo);

private:
  typedef std::pair<std::string, int> IntData;
  std::vector<IntData> int_;

  typedef std::pair<std::string, double> FloatData;
  std::vector<FloatData> float_;

  typedef std::pair<std::string, std::string> StringData;
  std::vector<StringData> string_, filename_;

  typedef std::pair<std::string, Floats> FloatsData;
  std::vector<FloatsData> floats_;

  typedef std::pair<std::string, Ints> IntsData;
  std::vector<IntsData> ints_;

  typedef std::pair<std::string, Strings> StringsData;
  std::vector<StringsData> strings_;
  std::vector<StringsData> filenames_;

  typedef std::pair<std::string, ParticleIndexes> ParticleIndexesData;
  std::vector<ParticleIndexesData> pis_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RESTRAINT_INFO_H */
