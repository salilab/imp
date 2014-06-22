/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Particle.h"
#include "IMP/base/log_macros.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/internal/PrefixStream.h"
#include "IMP/kernel/Decorator.h"

IMPKERNEL_BEGIN_NAMESPACE

Particle::Particle(kernel::Model *m, std::string name)
    : ModelObject(m, name), id_(base::get_invalid_index<ParticleIndexTag>()) {
  m->add_particle_internal(this);
}

Particle::Particle(kernel::Model *m)
    : ModelObject(m, "P%1%"), id_(base::get_invalid_index<ParticleIndexTag>()) {
  m->add_particle_internal(this);
}

namespace {
struct IdentityValue {
  template <class T>
  const T &operator()(const T &t) const {
    return t;
  }
};
struct NameValue {
  Model *m_;
  NameValue(kernel::Model *m) : m_(m) {}
  template <class T>
  std::string operator()(const T &t) const {
    std::ostringstream oss;
    oss << "\"" << m_->get_particle(t)->get_name() << "\"";
    return oss.str();
  }
};
struct DirectNameValue {
  template <class T>
  std::string operator()(const T &t) const {
    std::ostringstream oss;
    oss << "\"" << t->get_name() << "\"";
    return oss.str();
  }
};
template <class V>
struct SizeValue {
  V v_;
  SizeValue(V v = V()) : v_(v) {}
  template <class T>
  std::string operator()(const T &t) const {
    std::ostringstream oss;
    if (t.size() < 5) {
      oss << "[";
      for (unsigned int i = 0; i < t.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << v_(t[i]);
      }
    } else {
      oss << "length " << t.size();
    }
    return oss.str();
  }
};
template <class Keys, class GetValue>
void show_attributes(kernel::Model *m, const Keys &fks, ParticleIndex id,
                     std::string name, GetValue v,
                     internal::PrefixStream &preout) {
  if (!fks.empty()) {
    preout << name << " attributes:" << std::endl;
    preout.set_prefix("  ");
    for (typename Keys::const_iterator it = fks.begin(); it != fks.end();
         ++it) {
      preout << *it << ": " << v(m->get_attribute(*it, id, false)) << std::endl;
    }
    preout.set_prefix("");
  }
}
}

void Particle::show(std::ostream &out) const {
  internal::PrefixStream preout(&out);
  preout << "index: " << get_index()
         << (get_is_active() ? " (active)" : " (dead)");
  preout << std::endl;

  if (get_model()) {
    {
      FloatKeys fks = get_float_keys();
      if (!fks.empty()) {
        preout << "float attributes:" << std::endl;
        preout.set_prefix("  ");
        for (FloatKeys::const_iterator it = fks.begin(); it != fks.end();
             ++it) {
          FloatKey k = *it;
          preout << k << ": " << get_model()->get_attribute(k, id_, false);
          preout << " (" << get_model()->get_derivative(k, id_, false) << ") ";
          preout << (get_is_optimized(k) ? " (optimized)" : "");
          preout << std::endl;
        }
        preout.set_prefix("");
      }
    }
    show_attributes(get_model(), get_int_keys(), id_, "int", IdentityValue(),
                    preout);
    show_attributes(get_model(), get_string_keys(), id_, "string",
                    IdentityValue(), preout);
    show_attributes(
        get_model(),
        get_model()->internal::ParticleAttributeTable::get_attribute_keys(id_),
        id_, "particle", NameValue(get_model()), preout);
    show_attributes(
        get_model(),
        get_model()->internal::ObjectAttributeTable::get_attribute_keys(id_),
        id_, "object", DirectNameValue(), preout);
    show_attributes(
        get_model(),
        get_model()->internal::ParticlesAttributeTable::get_attribute_keys(id_),
        id_, "particles", SizeValue<NameValue>(NameValue(get_model())), preout);
    show_attributes(
        get_model(),
        get_model()->internal::ObjectsAttributeTable::get_attribute_keys(id_),
        id_, "objects", SizeValue<DirectNameValue>(), preout);
    show_attributes(
        get_model(),
        get_model()->internal::IntsAttributeTable::get_attribute_keys(id_), id_,
        "ints", SizeValue<IdentityValue>(), preout);
  }
}

void Particle::clear_caches() { get_model()->clear_particle_caches(id_); }

bool Particle::get_is_active() const {
  IMP_CHECK_OBJECT(this);
  return get_model() && get_model()->get_has_particle(get_index());
}

ParticleIndex Particle::get_index() const { return id_; }

void Particle::add_attribute(FloatKey name, const Float initial_value,
                             bool optimized) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(name, id_, initial_value);
  get_model()->set_is_optimized(name, id_, optimized);
}

void Particle::add_to_derivative(FloatKey key, Float value,
                                 const DerivativeAccumulator &da) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_to_derivative(key, id_, value, da);
}
void Particle::set_is_optimized(FloatKey k, bool tf) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->set_is_optimized(k, id_, tf);
}
void Particle::add_attribute(ParticleIndexKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->add_attribute(k, id_, v->get_index());
}
bool Particle::has_attribute(ParticleIndexKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_has_attribute(k, id_);
}
void Particle::set_value(ParticleIndexKey k, Particle *v) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->set_attribute(k, id_, v->get_index());
}
void Particle::remove_attribute(ParticleIndexKey k) {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  get_model()->remove_attribute(k, id_);
}
ParticleIndexKeys Particle::get_particle_keys() const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->internal::ParticleAttributeTable::get_attribute_keys(id_);
}

ParticleAdaptor::ParticleAdaptor(const Decorator &d)
    : m_(d.get_model()), pi_(d.get_particle_index()) {}

IMPKERNEL_END_NAMESPACE
