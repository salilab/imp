/**
 *  \file IMP/TripletDerivativeModifier.h
 *  \brief A Modifier on ParticleTripletsTemp
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_TRIPLET_DERIVATIVE_MODIFIER_H
#define IMPKERNEL_TRIPLET_DERIVATIVE_MODIFIER_H
IMPKERNEL_DEPRECATED_HEADER(2.24, "Use TripletModifier.h instead")

#include <IMP/kernel_config.h>
#include "DerivativeAccumulator.h"
#include "base_types.h"
#include "internal/container_helpers.h"
#include "TripletModifier.h"

IMPKERNEL_BEGIN_NAMESPACE
typedef TripletModifier TripletDerivativeModifier;

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_TRIPLET_DERIVATIVE_MODIFIER_H */
