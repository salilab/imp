/**
 *  \file internal/particle_triplet_helpers.h
 *  \brief A container for Triplets.
 *
 *  WARNING This file was generated from name_helpers.hpp
 *  in tools/maintenance/container_templates/kernel/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_TRIPLET_HELPERS_H
#define IMPKERNEL_INTERNAL_TRIPLET_HELPERS_H

#include "../kernel_config.h"
#include "../TripletContainer.h"
#include "../TripletModifier.h"
#include "../TripletScore.h"
#include "container_helpers.h"
#include <algorithm>

IMP_BEGIN_NAMESPACE
class Model;
IMP_END_NAMESPACE

IMP_BEGIN_INTERNAL_NAMESPACE

template <class S>
inline void call_apply_index(Model *m, const S *s,
                      const ParticleIndexTriplet& a) {
  s->S::apply_index(m, a);
}
inline void call_apply(Model *m, const TripletModifier *s,
                const ParticleIndexTriplet& a) {
  s->apply_index(m, a);
}
template <class S>
inline void call_apply_index(Model *m, const S *s,
                      const ParticleIndexTriplet& a,
                      DerivativeAccumulator *&da) {
  s->S::apply_index(m, a, da);
}

IMP_END_INTERNAL_NAMESPACE


#endif  /* IMPKERNEL_INTERNAL_TRIPLET_HELPERS_H */
