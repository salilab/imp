/**
 *  \file hierarchy_helpers.h
 *  \brief Helpers for the Hierarchy.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_HIERARCHY_HELPERS_H
#define IMPCORE_INTERNAL_HIERARCHY_HELPERS_H

#include <IMP/core/core_config.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
class HierarchyTraits;
IMPCORE_END_NAMESPACE

IMPCORE_BEGIN_INTERNAL_NAMESPACE

template <class K, class V>
struct MatchAttribute {
  K k_;
  V v_;
  MatchAttribute(K k, V v) : k_(k), v_(v) {}
  bool operator()(Particle *o) {
    if (!o->has_attribute(k_))
      return false;
    else
      return o->get_value(k_) == v_;
  }
};

template <class K0, class V0, class K1, class V1>
struct MatchAttributes {
  K0 k0_;
  V0 v0_;
  K1 k1_;
  V1 v1_;
  MatchAttributes(K0 k0, V0 v0, K1 k1, V1 v1)
      : k0_(k0), v0_(v0), k1_(k1), v1_(v1) {}
  bool operator()(Particle *o) {
    if (!o->has_attribute(k0_))
      return false;
    else if (o->get_value(k0_) != v0_)
      return false;
    else if (!o->has_attribute(k1_))
      return false;
    else if (o->get_value(k1_) != v1_)
      return false;
    return true;
  }
};

struct HierarchyData {
  ParticleIndexKey parent_key_;
  IntKey parent_index_key_;
  ObjectKey cache_key_;
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_HIERARCHY_HELPERS_H */
