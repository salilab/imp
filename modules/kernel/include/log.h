/**
 *  \file IMP/log.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_LOG_H
#define IMPKERNEL_LOG_H

#include "kernel_config.h"
#include <IMP/base/log.h>

IMP_BEGIN_NAMESPACE

using IMP::base::LogLevel;
using IMP::base::DEFAULT;
using IMP::base::SILENT;
using IMP::base::TERSE;
using IMP::base::VERBOSE;
using IMP::base::WARNING;
using IMP::base::MEMORY;
using IMP::base::PROGRESS;
using IMP::base::set_log_level;
using IMP::base::CreateLogContext;
using IMP::base::get_log_level;
using IMP::base::set_log_target;
IMP_END_NAMESPACE

#endif  /* IMPKERNEL_LOG_H */
