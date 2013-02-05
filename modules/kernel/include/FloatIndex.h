/**
 *  \file IMP/kernel/FloatIndex.h
 *  \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FLOAT_INDEX_H
#define IMPKERNEL_FLOAT_INDEX_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "particle_index.h"
#include <IMP/base/tuple_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

/** A FloatIndex identifies an optimized attribute in a model.
 */
IMP_NAMED_TUPLE_2(FloatIndex, FloatIndexes, ParticleIndex, particle,
                  FloatKey, key,);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_FLOAT_INDEX_H */
