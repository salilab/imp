/**
 *  \file kernel_version_info.h    \brief IMP kernel version information.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_KERNEL_VERSION_INFO_H
#define __IMP_KERNEL_VERSION_INFO_H

#include "../VersionInfo.h"

namespace IMP
{

namespace internal
{

//! Version and authorship of the IMP kernel.
extern IMPDLLEXPORT VersionInfo kernel_version_info;

} // namespace internal

} // namespace IMP

#endif  /* __IMP_KERNEL_VERSION_INFO_H */
