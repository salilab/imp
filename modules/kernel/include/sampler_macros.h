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


//! Define the basic things you need for a Sampler.
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Sampler::do_sample()

    \relatesalso IMP::Sampler
*/
#define IMP_SAMPLER(Name)                       \
  IMP_OBJECT(Name);                             \
protected:                                      \
 IMP_IMPLEMENT(ConfigurationSet* do_sample() const)


#endif  /* IMPKERNEL_SAMPLER_MACROS_H */
