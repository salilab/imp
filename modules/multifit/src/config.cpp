/**
 *  \file IMP/multifit/config.cpp
 *  \brief multifit module version information.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/multifit/config.h>
#include <IMP/VersionInfo.h>

IMPMULTIFIT_BEGIN_NAMESPACE


/** Return the version info for the module. Classes in the module
    can return this as their version info. */
const VersionInfo& get_module_version_info() {
    static VersionInfo vi("IMP.multifit development team", "SVN 3769:4264M");
    return vi;
}


IMPMULTIFIT_END_NAMESPACE
