/**
 *  \file IMP/InputAdaptor.h
 *  \brief Convenience class to accept multiple input types.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INPUT_ADAPTOR_H
#define IMPKERNEL_INPUT_ADAPTOR_H

#include <IMP/kernel_config.h>
#include "utility_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Convenience class to accept multiple input types.
/** An input adaptor is a convenience class that allows functions to accept a
    wider variety of inputs than they would otherwise. The InputAdaptor class
    itself should not be used directly; it is just provided for purposes of
    documentation.

    Input adaptor classes do not support output, comparisons or hashing.
 */
class InputAdaptor {
 protected:
  InputAdaptor() {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_INPUT_ADAPTOR_H */
