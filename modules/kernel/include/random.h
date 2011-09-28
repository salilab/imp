/**
 *  \file IMP/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include "kernel_config.h"
#include <IMP/base/random.h>

IMP_BEGIN_NAMESPACE
#ifndef SWIG
using base::RandomNumberGenerator;
using base::random_number_generator;
#endif
IMP_END_NAMESPACE

#endif  /* IMPKERNEL_RANDOM_H */
