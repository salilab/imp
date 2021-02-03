/**
 *  \file internal/log.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_EAT_H
#define IMPKERNEL_INTERNAL_EAT_H

#include <IMP/kernel_config.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
// fake usage of variable (for IMP_UNUSED)
template <class T>
void eat(const T&) {}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_EAT_H */
