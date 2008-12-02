/**
 *  \file hierarchy_helpers.h
 *  \brief Helpers for the HierarchyDecorator.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_HIERARCHY_HELPERS_H
#define IMPCORE_INTERNAL_HIERARCHY_HELPERS_H

#include "../core_exports.h"
#include "../macros.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE
class HierarchyDecorator;
IMPCORE_END_NAMESPACE

IMPCORE_BEGIN_INTERNAL_NAMESPACE

// needs to be external to keep swig happy
struct IMPCOREEXPORT ChildArrayTraits
{
  static ParticleKey parent_key_;
  static IntKey parent_index_key_;

  typedef ParticleKey Key;
  typedef Particle* Value;
  template <class HD>
  static void on_add(Particle * p, HD d, unsigned int i) {
    d.get_particle()->add_attribute(parent_key_, p);
    d.get_particle()->add_attribute(parent_index_key_, i);
  }
  static void on_change(Particle *, Particle* p, unsigned int oi,
                        unsigned int ni) {
    p->set_value(parent_index_key_, ni);
  }
  template <class HD>
  static void on_remove(Particle *, HD d) {
    d.get_particle()->remove_attribute(parent_index_key_);
    d.get_particle()->remove_attribute(parent_key_);
  }
  template <class HD>
  static Particle *get_value(HD d) {
    return d.get_particle();
  }
  template <class HD>
  static unsigned int get_index(Particle *, HD d) {
    return d.get_parent_index();
  }
};


template <class K, class V>
struct MatchAttribute
{
  K k_;
  V v_;
  MatchAttribute(K k, V v): k_(k), v_(v){}
  bool operator()(Particle *o) {
    if (!o->has_attribute(k_)) return false;
    else return o->get_value(k_) == v_;
  }
};


template <class K0, class V0, class K1, class V1>
struct MatchAttributes
{
  K0 k0_;
  V0 v0_;
  K1 k1_;
  V1 v1_;
  MatchAttributes(K0 k0, V0 v0,
                  K1 k1, V1 v1): k0_(k0), v0_(v0),
                                 k1_(k1), v1_(v1){
                                 }
  bool operator()(Particle *o) {
    if (!o->has_attribute(k0_)) return false;
    else if (o->get_value(k0_) != v0_) return false;
    else if (!o->has_attribute(k1_)) return false;
    else if (o->get_value(k1_) != v1_) return false;
    return true;
  }
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_HIERARCHY_HELPERS_H */
