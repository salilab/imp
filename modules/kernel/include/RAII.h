/**
 *  \file IMP/base/RAII.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RAI_I_H
#define IMPKERNEL_RAI_I_H

#include <IMP/kernel_config.h>
#include "NonCopyable.h"
#include "utility_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
/** An RAII class (Resource Acquisition Is
       Initialization) provides a way of
       temporarily changing something and undoing the
       change when the RAII object
       is destroyed. Examples include reference counting
       and changing the log level
       where you want to do something (increase the amount
       of logging), but ensure
       that when your function exits, things are put back
       to how they were before.
   */
class RAII : public NonCopyable {
 protected:
  RAII() {}
};
IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RAI_I_H */
