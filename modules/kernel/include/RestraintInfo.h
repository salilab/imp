/**
 *  \file IMP/RestraintInfo.h
 *  \brief Report key:value information on restraints
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_INFO_H
#define IMPKERNEL_RESTRAINT_INFO_H

#include <IMP/kernel_config.h>
#include <IMP/Object.h>
#include <IMP/types.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Report key:value information on restraints
/** These objects are typically returned by Restraint::get_static_info()
    or Restraint::get_dynamic_info() and are used to report information
    about a Restraint instance as a set of key:value pairs. The primary
    purpose is to allow restraints to be written to files, such as RMF.
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

  IMP_OBJECT_METHODS(RestraintInfo);

private:
  typedef std::pair<std::string, int> IntData;
  std::vector<IntData> int_;

  typedef std::pair<std::string, double> FloatData;
  std::vector<FloatData> float_;

  typedef std::pair<std::string, Floats> FloatsData;
  std::vector<FloatsData> floats_;

  typedef std::pair<std::string, Strings> StringsData;
  std::vector<StringsData> filenames_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RESTRAINT_INFO_H */
