/**
 *  \file IMP/Particle.h
 *  \brief Classes to handle individual model particles.
 *         (Note that implementation of inline functions is in internal)
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_H
#define IMPKERNEL_PARTICLE_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "DerivativeAccumulator.h"
#include "Model.h"
#include "ModelObject.h"
#include "particle_index.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include <IMP/Object.h>
#include <IMP/base_utility.h>
#include <IMP/Pointer.h>
#include <IMP/check_macros.h>
#include <utility>

IMPKERNEL_BEGIN_NAMESPACE

//class Model;
class Changed;
class SaveOptimizeds;

//! Class to handle individual particles of a Model object.
/** A particle is a lightweight object that serves as a place holder for the
    particle index in a given Model object. The Model object itself stores
    the particle attributes of various types:
    Float, Int, String, Object, and WeakObject, and any data concerning the 
    optimization of these particles by Optimizer and Simulator classes.

*/
class IMPKERNELEXPORT Particle : public ModelObject {
 private:
// doxygen produces funny docs for these things
#ifndef IMP_DOXYGEN
  friend class Model;
#endif
  ParticleIndex id_;
  IMP_OBJECT_METHODS(Particle);

 public:
  //! Construct a particle and add it to the Model
  Particle(Model *m, std::string name);

  //! Construct a particle and add it to the Model
  Particle(Model *m);

#ifndef IMP_DOXYGEN

#define IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(UCName, lcname, Value) \
  inline void add_attribute(UCName##Key name, Value initial_value);           \
  inline void remove_attribute(UCName##Key name);                             \
  inline bool has_attribute(UCName##Key name) const;                          \
  inline Value get_value(UCName##Key name) const;                             \
  inline void set_value(UCName##Key name, Value value);                       \
  inline void add_cache_attribute(UCName##Key name, Value value);             \
  inline UCName##Keys get_##lcname##_keys() const

  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Float, float, Float);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Floats, floats, Floats);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Int, int, Int);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Ints, ints, Ints);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(String, string, String);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Object, object, Object *);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(WeakObject, weak_object, Object *);

  /** @name Float Attributes
       Float attributes can be optimized, meaning an Optimizer class is
       allowed to change their value in order to improve the score.
       As a result, there are a number of specialized methods to manipulate
       the value of float attributes.

       All distances are assumed to be in angstroms
       and derivatives in kcal/mol/angstrom. This is not enforced.
   */
  /*@{*/
  /** add attribute name to the attributes table of this particle

      @param name attribute key
      @param initial_value initial value for the attribute
      @param optimized whether to flag this attribute as optimized
  */
  void add_attribute(FloatKey name, const Float initial_value, bool optimized);

  /** Adds value to the derivatives table of the
      specified particle attribute

      @param key the attribute key whose derivative is updated
      @param value the derivative value to be added
      @param da a derivative accumulator for reweighting derivatives
  */
  void add_to_derivative(FloatKey key, Float value,
                         const DerivativeAccumulator &da);

  void set_is_optimized(FloatKey k, bool tf);

  //! returns true if key k is marked by model as optimized
  inline bool get_is_optimized(FloatKey k) const;

  //! returns the derivative of the specified particle attribute
  inline Float get_derivative(FloatKey k) const;
  /** @} */

  /** \name Particle attributes
      @{
  */
  void add_attribute(ParticleIndexKey k, Particle *v);
  bool has_attribute(ParticleIndexKey k);
  void set_value(ParticleIndexKey k, Particle *v);
  inline Particle *get_value(ParticleIndexKey k) const;
  void remove_attribute(ParticleIndexKey k);
  ParticleIndexKeys get_particle_keys() const;
  /** @} */

  //! Print out all the attributes
  void show(std::ostream &out = std::cout) const;

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
  */
  bool get_is_active() const;
#endif

  //! returns the particle index of this particle in its model
  ParticleIndex get_index() const;

#if !defined(IMP_DOXYGEN)
  void clear_caches() IMP_OVERRIDE;
#endif
 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE IMP_FINAL {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE IMP_FINAL {
    return ModelObjectsTemp();
  }
};

// for swig
class Decorator;

/** Take Decorator or Particle. */
class IMPKERNELEXPORT ParticleAdaptor : public InputAdaptor {
  Model *m_;
  ParticleIndex pi_;

 public:
  ParticleAdaptor() : m_(nullptr), pi_() {}
  ParticleAdaptor(Particle *p) : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(const Decorator &d);
#ifndef SWIG
  ParticleAdaptor(IMP::Pointer<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(IMP::WeakPointer<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(IMP::PointerMember<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
#endif
  Model *get_model() const { return m_; }
  ParticleIndex get_particle_index() const { return pi_; }
};


/****************** Inline methods ***************/

#ifndef IMP_DOXYGEN

bool Particle::get_is_optimized(FloatKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_is_optimized(k, id_);
}

Float Particle::get_derivative(FloatKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_derivative(k, id_);
}

Particle *Particle::get_value(ParticleIndexKey k) const {
  IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");
  return get_model()->get_particle(get_model()->get_attribute(k, id_));
}

#define IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(UCName, lcname, Value)                \
  void Particle::add_attribute(UCName##Key name, Value initial_value) {       \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    get_model()->add_attribute(name, id_, initial_value);                     \
  }                                                                           \
  void Particle::remove_attribute(UCName##Key name) {                         \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    get_model()->remove_attribute(name, id_);                                 \
  }                                                                           \
  bool Particle::has_attribute(UCName##Key name) const {                      \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    return get_model()->get_has_attribute(name, id_);                         \
  }                                                                           \
  Value Particle::get_value(UCName##Key name) const {                         \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    return get_model()->get_attribute(name, id_);                             \
  }                                                                           \
  void Particle::set_value(UCName##Key name, Value value) {                   \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    get_model()->set_attribute(name, id_, value);                             \
  }                                                                           \
  UCName##Keys Particle::get_##lcname##_keys() const {                        \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    return get_model()->internal::UCName##AttributeTable::get_attribute_keys( \
        id_);                                                                 \
  }                                                                           \
  void Particle::add_cache_attribute(UCName##Key name, Value value) {         \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");              \
    return get_model()->add_cache_attribute(name, id_, value);                \
  }

IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Float, float, Float);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Floats, floats, Floats);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Int, int, Int);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Ints, ints, Ints);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(String, string, String);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(Object, object, Object *);
IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(WeakObject, weak_object, Object *);

#endif // DOXYGEN

IMPKERNEL_END_NAMESPACE
#endif /* IMPKERNEL_PARTICLE_H */
