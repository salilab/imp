/**
 *  \file IMP/SingletonDerivativeModifier.h
 *  \brief A Modifier on ParticlesTemp
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_SINGLETON_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_SINGLETON_DERIVATIVE_MODIFIER_H
IMPKERNEL_DEPRECATED_HEADER(2.24, "Use SingletonModifier.h instead")

#include <IMP/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "internal/container_helpers.h"
#include "SingletonModifier.h"

IMPKERNEL_BEGIN_NAMESPACE
typedef SingletonModifier SingletonDerivativeModifier;

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SINGLETON_DERIVATIVE_MODIFIER_H */
