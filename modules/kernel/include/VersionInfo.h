/**
 *  \file IMP/VersionInfo.h   \brief Version and authorship of IMP objects.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_VERSION_INFO_H
#define IMPKERNEL_VERSION_INFO_H

#include <IMP/kernel_config.h>
#include "exception.h"
#include "comparison_macros.h"
#include "check_macros.h"
#include "showable_macros.h"
#include "value_macros.h"
#include "Value.h"
#include <cereal/access.hpp>
#include <iostream>

IMPKERNEL_BEGIN_NAMESPACE

//! Version and module information for Objects
/** All IMP::Object -derived objects have a method
    IMP::Object::get_version_info() returning such an object. The
    version info allows one to determine the module and version of
    all restraints used to help creating reproducible results.
 */
class IMPKERNELEXPORT VersionInfo : public Value {
 public:
  //! Create a VersionInfo object with the given module and version.
  VersionInfo(std::string module, std::string version);

  VersionInfo() {}

  std::string get_module() const { return module_; }

  std::string get_version() const { return version_; }
  IMP_SHOWABLE_INLINE(VersionInfo, {
    IMP_USAGE_CHECK(!module_.empty(),
                    "Attempting to use uninitialized version info");
    out << module_ << " " << version_;
  });
  IMP_COMPARISONS_2(VersionInfo, module_, version_);

 private:
  std::string module_, version_;
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(module_, version_);
  }
};
IMP_VALUES(VersionInfo, VersionInfos);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_VERSION_INFO_H */
