/**
 *  \file IMP/kernel/log.h    \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_LOG_H
#define IMPKERNEL_LOG_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/log.h>
#include <IMP/base/log_macros.h>

IMPKERNEL_BEGIN_NAMESPACE
#ifndef IMP_DOXYGEN
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
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_LOG_H */
