/**
 *  \file IMP/exception.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_EXCEPTION_H
#define IMPKERNEL_EXCEPTION_H

#include "kernel_config.h"
#include <IMP/base/exception.h>
#include <IMP/base/SetLogState.h>

IMP_BEGIN_NAMESPACE

using base::ValueException;
using base::IOException;
using base::IndexException;
using base::ModelException;
using base::Exception;
using base::USAGE;
using base::USAGE_AND_INTERNAL;
using base::NONE;
using base::SetLogState;
using base::WarningContext;
using base::set_print_exceptions;

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_EXCEPTION_H */
