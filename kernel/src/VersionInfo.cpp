/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/VersionInfo.h"

IMP_BEGIN_NAMESPACE

VersionInfo::VersionInfo(std::string module,
                         std::string version) : module_(module),
                                                version_(version) {}
IMP_END_NAMESPACE
