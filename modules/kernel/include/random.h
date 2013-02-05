/**
 *  \file IMP/kernel/random.h
 *  \brief For backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/random.h>

IMPKERNEL_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
using base::RandomNumberGenerator;
using base::random_number_generator;
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_RANDOM_H */
