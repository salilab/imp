/**
 *  \file IMP/PairDerivativeModifier.h
 *  \brief A Modifier on ParticlePairsTemp
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H
IMPKERNEL_DEPRECATED_HEADER(2.24, "Use PairModifier.h instead")

#include <IMP/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "internal/container_helpers.h"
#include "PairModifier.h"

IMPKERNEL_BEGIN_NAMESPACE
typedef PairModifier PairDerivativeModifier;

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_PAIR_DERIVATIVE_MODIFIER_H */
