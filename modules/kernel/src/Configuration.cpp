/**
 *  \file Configuration.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Configuration.h"
#include "IMP/internal/utility.h"
#include "IMP/dependency_graph.h"

IMPKERNEL_BEGIN_NAMESPACE

#define IMP_CONFIG_FOREACH(OPERATION)                                          \
  OPERATION(floats, Float, Float, are_equal);                                  \
  OPERATION(float_lists, Floats, Floats, are_equal);                           \
  OPERATION(strings, String, String, are_equal);                               \
  OPERATION(ints, Int, Int, are_equal);                                        \
  OPERATION(objects, Object, Object, are_equal);                               \
  OPERATION(weak_objects, WeakObject, WeakObject, are_equal);                  \
  OPERATION(int_lists, Ints, Ints, are_equal);                                 \
  OPERATION(object_lists, Objects, Objects, are_equal);                        \
  OPERATION(particles, Particle, Particle, are_equal);                         \
  OPERATION(particle_lists, Particles, Particles, are_equal);                  \
  OPERATION(vector3ds, Vector3D, Vector3D, vector_equal);                      \
  OPERATION(vector4ds, Vector4D, Vector4D, vector_equal);                      \
  OPERATION(sparse_strings, SparseString, SparseString, are_equal);            \
  OPERATION(sparse_ints, SparseInt, SparseInt, are_equal);                     \
  OPERATION(sparse_floats, SparseFloat, SparseFloat, are_equal);               \
  OPERATION(sparse_particles, SparseParticleIndex, SparseParticle, are_equal); \
  OPERATION(vector3d_derivs, Vector3DDeriv, Vector3DDeriv, vector_equal);      \
  OPERATION(vector4d_derivs, Vector4DDeriv, Vector4DDeriv, vector_equal)

Configuration::Configuration(Model *m, std::string name)
    : Object(name), model_(m) {
#define IMP_CONFIG_COPY(name, Name, TablePre, Equal) name##_ = *m;

  IMP_CONFIG_FOREACH(IMP_CONFIG_COPY);
}

namespace {
// use exact equality for VectorD
template <class T>
bool vector_equal(const T&a, const T&b) {
  return std::equal(a.begin(), a.end(), b.begin());
}

template <class T>
bool are_equal(const T &a, const T &b) {
  return a == b;
}
template <class T>
bool are_not_equal(const Vector<T> &a, const Vector<T> &b) {
  if (a.size() != b.size())
    return true;
  else {
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (!are_equal(a[i], b[i])) return false;
    }
  }
  return true;
}
}

#define IMP_CONFIG_CHECK_COPY(name, Name, TablePre, Equal)                    \
  {                                                                            \
    const internal::TablePre##AttributeTable &mtable =                        \
        static_cast<internal::TablePre##AttributeTable &>(*model_);           \
    for (unsigned int i = 0; i < mtable.size(); ++i) {                         \
      if (add_remove_found) break;                                             \
      for (unsigned int j = 0; j < mtable.size(i); ++j) {                      \
        if (mtable.get_has_attribute(Name##Key(i), ParticleIndex(j)) !=        \
            base->name##_.get_has_attribute(Name##Key(i), ParticleIndex(j))) { \
          IMP_WARN("Falling back on dumb configuration saving.");              \
          add_remove_found = true;                                             \
          break;                                                               \
        }                                                                      \
        if (mtable.get_has_attribute(Name##Key(i), ParticleIndex(j)) &&        \
            !Equal(mtable.get_attribute(Name##Key(i), ParticleIndex(j)),       \
                       base->name##_.get_attribute(Name##Key(i),               \
                                                   ParticleIndex(j)))) {       \
          name##_.add_attribute(                                               \
              Name##Key(i), ParticleIndex(j),                                  \
              mtable.get_attribute(Name##Key(i), ParticleIndex(j)));           \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

Configuration::Configuration(Model *m, Configuration *base,
                             std::string name)
    : Object(name), model_(m), base_(base) {
  bool add_remove_found = false;
  IMP_CONFIG_FOREACH(IMP_CONFIG_CHECK_COPY);

  if (add_remove_found) {
    // fall back on dumb way
    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY);
    base_ = nullptr;
  }
}

#define IMP_CONFIG_COPY_BACK_BASE(name, Name, TablePre, Equal)                \
  {                                                                           \
    internal::TablePre##AttributeTable &mtable =                             \
        static_cast<internal::TablePre##AttributeTable &>(*model_);          \
    for (unsigned int i = 0; i < mtable.size(); ++i) {                        \
      for (unsigned int j = 0; j < mtable.size(i); ++j) {                     \
        if (mtable.get_has_attribute(Name##Key(i), ParticleIndex(j))) {       \
          if (name##_.get_has_attribute(Name##Key(i), ParticleIndex(j))) {    \
            mtable.set_attribute(                                             \
                Name##Key(i), ParticleIndex(j),                               \
                name##_.get_attribute(Name##Key(i), ParticleIndex(j)));       \
          } else {                                                            \
            mtable.set_attribute(                                             \
                Name##Key(i), ParticleIndex(j),                               \
                base->name##_.get_attribute(Name##Key(i), ParticleIndex(j))); \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    }                                                                         \
  }

#define IMP_CONFIG_COPY_BACK(name, Name, TablePre, Equal) \
  static_cast<internal::TablePre##AttributeTable &>(*model_) = ncthis->name##_

void Configuration::load_configuration() const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  // workaround for weird mac os and boost 1.48 bug
  Configuration *ncthis = const_cast<Configuration *>(this);
  if (!base_) {
    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY_BACK);
  } else {
    Configuration *base = dynamic_cast<Configuration *>(base_.get());

    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY_BACK_BASE);
  }
}

#define IMP_CONFIG_SWAP(name, Name, TablePre, Equal) \
  swap(static_cast<internal::TablePre##AttributeTable &>(*model_), name##_)

void Configuration::swap_configuration() {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(!base_, "Cannot use swap_configuration() was a base was used"
                              << " for saving the configuration. Sorry.");
  set_was_used(true);
  using std::swap;
  IMP_CONFIG_FOREACH(IMP_CONFIG_SWAP);
}

uint32_t Configuration::get_model_id() const {
  return model_->get_unique_id();
}

void Configuration::set_model_from_id(uint32_t model_id) {
  Model *m = Model::get_by_unique_id(model_id);
  if (!m) {
    IMP_THROW("Cannot unserialize Configuration as it refers to a "
              "Model that does not exist", ValueException);
  } else {
    model_ = m;
  }
}

IMPKERNEL_END_NAMESPACE
