/**
 *  \file Configuration.cpp
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Configuration.h"
#include "IMP/kernel/internal/utility.h"
#include "IMP/kernel/dependency_graph.h"
#include "IMP/base//set.h"

IMPKERNEL_BEGIN_NAMESPACE

#define IMP_CONFIG_FOREACH(OPERATION)               \
  OPERATION(floats, Float);                         \
  OPERATION(strings, String);                       \
  OPERATION(ints, Int);                             \
  OPERATION(objects, Object);                       \
  OPERATION(weak_objects, WeakObject);              \
  OPERATION(int_lists, Ints);                       \
  OPERATION(object_lists, Objects);                  \
  OPERATION(particles, Particle);                    \
  OPERATION(particle_lists, Particles)

Configuration::Configuration(Model *m, std::string name): Object(name),
  model_(m){
#define IMP_CONFIG_COPY(name, Name) name##_=*m;

  IMP_CONFIG_FOREACH(IMP_CONFIG_COPY);
}

namespace {
  template <class T>
  bool are_equal(const T &a, const T &b) {
    return a == b;
  }
  template <class T>
  bool are_not_equal(const base::Vector<T> &a, const base::Vector<T> &b) {
    if (a.size() != b.size()) return true;
    else {
      for (unsigned int i=0; i< a.size(); ++i) {
        if (!are_equal(a[i], b[i])) return false;
      }
    }
    return true;
  }
}

#define IMP_CONFIG_CHECK_COPY(name, Name)                               \
  {                                                                     \
    const internal::Name##AttributeTable& mtable                        \
      =static_cast<internal::Name##AttributeTable&>(*model_);           \
    for (unsigned int i=0; i< mtable.size(); ++i) {                     \
      if (add_remove_found) break;                                      \
      for (unsigned int j=0; j< mtable.size(i); ++j) {                  \
        if (mtable.get_has_attribute(Name##Key(i),                      \
                                     ParticleIndex(j))                  \
            != base->name##_.get_has_attribute(Name##Key(i),            \
                                               ParticleIndex(j))) {     \
          IMP_WARN("Falling back on dumb configuration saving.");       \
          add_remove_found=true;                                        \
          break;                                                        \
        }                                                               \
        if (mtable.get_has_attribute(Name##Key(i),                      \
                                     ParticleIndex(j))                  \
            && !are_equal(mtable.get_attribute(Name##Key(i),            \
                                              ParticleIndex(j)),        \
                         base->name##_.get_attribute(Name##Key(i),      \
                                                     ParticleIndex(j)))) { \
          name##_.add_attribute(Name##Key(i),                           \
                                ParticleIndex(j),                       \
                                mtable.get_attribute(Name##Key(i),      \
                                                     ParticleIndex(j))); \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }


Configuration::Configuration(Model *m, Configuration *base,
                             std::string name): Object(name),
                                                model_(m),
                                                base_(base){
  bool add_remove_found=false;
  IMP_CONFIG_FOREACH(IMP_CONFIG_CHECK_COPY);

  if (add_remove_found) {
    // fall back on dumb way
    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY);
    base_=nullptr;
  }
}



#define IMP_CONFIG_COPY_BACK_BASE(name, Name)                           \
  {\
    internal::Name##AttributeTable& mtable                              \
      =static_cast<internal::Name##AttributeTable&>(*model_);           \
    for (unsigned int i=0; i< mtable.size(); ++i) {                     \
      for (unsigned int j=0; j< mtable.size(i); ++j) {                  \
        if (mtable.get_has_attribute(Name##Key(i),                      \
                                     ParticleIndex(j))) {               \
          if (name##_.get_has_attribute(Name##Key(i),                   \
                                        ParticleIndex(j))) {            \
            mtable.set_attribute(Name##Key(i),                          \
                                 ParticleIndex(j),                      \
                                 name##_.get_attribute(Name##Key(i),    \
                                                       ParticleIndex(j))); \
          } else {                                                      \
            mtable.set_attribute(Name##Key(i),                          \
                                 ParticleIndex(j),                      \
                                 base->name##_.get_attribute(Name##Key(i), \
                                                         ParticleIndex(j))); \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
    }

#define IMP_CONFIG_COPY_BACK(name, Name)                                \
    static_cast<internal::Name##AttributeTable&>(*model_)= ncthis->name##_

void Configuration::load_configuration() const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  // workaround for weird mac os and boost 1.48 bug
  Configuration *ncthis= const_cast<Configuration*>(this);
  if (!base_) {
    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY_BACK);
  } else {
    Configuration *base= dynamic_cast<Configuration*>(base_.get());

    IMP_CONFIG_FOREACH(IMP_CONFIG_COPY_BACK_BASE);
  }
}

#define IMP_CONFIG_SWAP(name, Name)                                     \
    swap(static_cast<internal::Name##AttributeTable&>(*model_), name##_)

void Configuration::swap_configuration() {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(!base_, "Cannot use swap_configuration() was a base was used"
                  << " for saving the configuration. Sorry.");
  set_was_used(true);
  using std::swap;
  IMP_CONFIG_FOREACH(IMP_CONFIG_SWAP);
}


void Configuration::do_show(std::ostream &out) const {
  out <<  "configuration" << std::endl;
}

IMPKERNEL_END_NAMESPACE
