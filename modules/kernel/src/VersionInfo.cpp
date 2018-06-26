/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/VersionInfo.h"

IMPKERNEL_BEGIN_NAMESPACE
VersionInfo::VersionInfo(std::string module, std::string version)
    : module_(module), version_(version) {
  IMP_USAGE_CHECK(!module.empty() && !version.empty(),
                  "The module and version must not be empty.");
}
IMPKERNEL_END_NAMESPACE
