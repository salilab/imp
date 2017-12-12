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

  IMP_OBJECT_METHODS(RestraintInfo);

private:
  typedef std::pair<std::string, Floats> FloatsData;
  std::vector<FloatsData> floats_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RESTRAINT_INFO_H */
