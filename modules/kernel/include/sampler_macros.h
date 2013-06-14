/**
 *  \file IMP/kernel/sampler_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SAMPLER_MACROS_H
#define IMPKERNEL_SAMPLER_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/doxygen_macros.h>
#include <IMP/base/object_macros.h>
#include "Sampler.h"


/** \deprecated_at{2.1} Declare the needed methods directly.
*/
#define IMP_SAMPLER(Name)                       \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the needed functions directly."); \
  IMP_OBJECT_NO_WARNING(Name);                             \
protected:                                      \
 IMP_IMPLEMENT(ConfigurationSet* do_sample() const)


#endif  /* IMPKERNEL_SAMPLER_MACROS_H */
