/**
 *  \file VersionInfo.h   \brief Version and authorship of IMP objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_VERSION_INFO_H
#define __IMP_VERSION_INFO_H

#include "IMP_config.h"

#include <iostream>
#include <string>

IMP_BEGIN_NAMESPACE

//! Version and authorship of IMP objects
class IMPDLLEXPORT VersionInfo
{
public:
  VersionInfo(std::string author, std::string version) : author_(author),
                                                         version_(version) {}

  //! Default constructor
  VersionInfo() : author_("unknown"), version_("unknown") {}

  //! \return author of this object.
  std::string get_author() const { return author_; }

  //! \return version string of this object.
  std::string get_version() const { return version_; }

  //! Print version information to a stream.
  void show(std::ostream &out=std::cout) const {
    out << "version: " << version_ << std::endl;
    out << "author: " << author_ << std::endl;
  }

protected:
  std::string author_, version_;
};

IMP_END_NAMESPACE

#endif  /* __IMP_VERSION_INFO_H */
