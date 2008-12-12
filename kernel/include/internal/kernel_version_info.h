/**
 *  \file kernel_version_info.h    \brief IMP kernel version information.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_KERNEL_VERSION_INFO_H
#define IMP_KERNEL_VERSION_INFO_H

#include "../VersionInfo.h"

IMP_BEGIN_NAMESPACE

namespace internal
{

//! Version and authorship of the IMP kernel.
extern IMPEXPORT VersionInfo kernel_version_info;

} // namespace internal

IMP_END_NAMESPACE

#endif  /* IMP_KERNEL_VERSION_INFO_H */
