/**
 *  \file IMP/QuadDerivativeModifier.h
 *  \brief A Modifier on ParticleQuadsTemp
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H
IMPKERNEL_DEPRECATED_HEADER(2.24, "Use QuadModifier.h instead")

#include <IMP/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "internal/container_helpers.h"
#include "QuadModifier.h"

IMPKERNEL_BEGIN_NAMESPACE
typedef QuadModifier QuadDerivativeModifier;

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_QUAD_DERIVATIVE_MODIFIER_H */
