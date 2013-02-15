/**
 *  \file IMP/kernel/exception.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_EXCEPTION_H
#define IMPKERNEL_EXCEPTION_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/exception.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/SetLogState.h>
#include <IMP/base/WarningContext.h>

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
using base::ValueException;
using base::IOException;
using base::IndexException;
using base::ModelException;
using base::EventException;
using base::Exception;
using base::USAGE;
using base::USAGE_AND_INTERNAL;
using base::NONE;
using base::SetLogState;
using base::WarningContext;
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_EXCEPTION_H */
