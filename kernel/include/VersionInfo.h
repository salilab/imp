/**
 *  \file VersionInfo.h   \brief Version and authorship of IMP objects.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_VERSION_INFO_H
#define IMP_VERSION_INFO_H

#include "utility.h"
#include "exception.h"

#include <iostream>
#include <string>

IMP_BEGIN_NAMESPACE

//! Version and authorship of IMP objects
/** All IMP::Object-derived objects have such info, allowing one to dump
    out the version of all the restraints used.
 */
class IMPEXPORT VersionInfo
{
  typedef VersionInfo This;
public:
  //! Create a VersionInfo object with the given author and version.
  VersionInfo(std::string module, std::string version);

  VersionInfo() {}

  std::string get_module() const { return module_; }

  std::string get_version() const { return version_; }

  //! Print version information to a stream.
  void show(std::ostream &out=std::cout) const {
    IMP_USAGE_CHECK(!module_.empty(),
                    "Attempting to use uninitialized version info");
    out << module_ << " " << version_;
  }
  IMP_COMPARISONS_2(module_, version_);
private:
  std::string module_, version_;
};

IMP_OUTPUT_OPERATOR(VersionInfo);

IMP_END_NAMESPACE

#endif  /* IMP_VERSION_INFO_H */
