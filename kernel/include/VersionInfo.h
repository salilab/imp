/**
 *  \file VersionInfo.h   \brief Version and authorship of IMP objects.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_VERSION_INFO_H
#define IMP_VERSION_INFO_H

#include "utility.h"
#include "exception.h"
#include "macros.h"

#include <iostream>
#include <vector>

IMP_BEGIN_NAMESPACE

//! Version and module information for Objects
/** All IMP::Object -derived objects have a method
    IMP::Object::get_version_info() returning such an object. The
    version info allows one to determine the module and version of
    all restraints used to help creating reproducable results.
 */
class IMPEXPORT VersionInfo
{
public:
  IMP_NO_DOXYGEN(typedef VersionInfo This);
  //! Create a VersionInfo object with the given module and version.
  VersionInfo(std::string module, std::string version);

  VersionInfo() {}

  std::string get_module() const { return module_; }

  std::string get_version() const { return version_; }
  IMP_SHOWABLE_INLINE(VersionInfo, {
      IMP_USAGE_CHECK(!module_.empty(),
                      "Attempting to use uninitialized version info");
      out << module_ << " " << version_;
    }
    );
  IMP_COMPARISONS_2(module_, version_);
private:
  std::string module_, version_;
};

IMP_OUTPUT_OPERATOR(VersionInfo);

IMP_VALUES(VersionInfo, VersionInfos);

IMP_END_NAMESPACE

#endif  /* IMP_VERSION_INFO_H */
