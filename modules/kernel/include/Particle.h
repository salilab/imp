/**
 *  \file IMP/kernel/Particle.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_H
#define IMPKERNEL_PARTICLE_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <IMP/base/Object.h>
#include <IMP/base/utility.h>
#include "Key.h"
#include "internal/AttributeTable.h"
#include "DerivativeAccumulator.h"
#include <IMP/base/Pointer.h>
#include "ModelObject.h"
#include "particle_index.h"
#include <utility>

IMPKERNEL_BEGIN_NAMESPACE

class Model;
class Changed;
class SaveOptimizeds;

//! Class to handle individual model particles.
/** At this point a Particle should only be considered as a placeholder for the
    ParticleIndex, accessed through the get_index() method.
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
  Particle(kernel::Model *m, std::string name);

  //! Construct a particle and add it to the Model
  Particle(kernel::Model *m);

#ifndef IMP_DOXYGEN

#define IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(UCName, lcname, Value) \
  void add_attribute(UCName##Key name, Value initial_value);           \
  void remove_attribute(UCName##Key name);                             \
  bool has_attribute(UCName##Key name) const;                          \
  Value get_value(UCName##Key name) const;                             \
  void set_value(UCName##Key name, Value value);                       \
  void add_cache_attribute(UCName##Key name, Value value);             \
  UCName##Keys get_##lcname##_keys() const

  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Float, float, Float);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Int, int, Int);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(String, string, String);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(Object, object, Object *);
  IMP_KERNEL_PARTICLE_ATTRIBUTE_TYPE_DECL(WeakObject, weak_object, Object *);

  /** @name Float Attributes
       Float attributes can be optimized, meaning the optimizer is
       allowed to change their value in order to improve the score.
       As a result, there are a number of extra methods to manipulate
       them.

       All distances are assumed to be in angstroms
       and derivatives in kcal/mol angstrom. This is not enforced.
   */
  /*@{*/
  /** add attribute name to the attributes table of this particle

      @param name attribute key
      @param inital_value initial value for the attribute
      @param optimized whether to flag this attribute as optimized
  */
  void add_attribute(FloatKey name, const Float initial_value, bool optimized);

  /** adds a derivative value to the derivatives table of this particle

      @param key the attribute key whose derivative is updates
      @param value the derivative value to be added
      @param da a derivative accumulator for reweighting derivatives
  */
  void add_to_derivative(FloatKey key, Float value,
                         const DerivativeAccumulator &da);

  void set_is_optimized(FloatKey k, bool tf);

  bool get_is_optimized(FloatKey k) const;

  Float get_derivative(FloatKey name) const;
  /** @} */

  /** \name Particle attributes
      @{
  */
  void add_attribute(ParticleIndexKey k, Particle *v);
  bool has_attribute(ParticleIndexKey k);
  void set_value(ParticleIndexKey k, Particle *v);
  Particle *get_value(ParticleIndexKey k) const;
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

  ParticleIndex get_index() const;

#if !defined(IMP_DOXYGEN)
  void clear_caches();
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
class IMPKERNELEXPORT ParticleAdaptor : public base::InputAdaptor {
  Model *m_;
  ParticleIndex pi_;

 public:
  ParticleAdaptor() : m_(nullptr), pi_() {}
  ParticleAdaptor(Particle *p) : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(const Decorator &d);
#ifndef SWIG
  ParticleAdaptor(IMP::base::Pointer<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(IMP::base::WeakPointer<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
  ParticleAdaptor(IMP::base::PointerMember<Particle> p)
      : m_(p->get_model()), pi_(p->get_index()) {}
#endif
  Model *get_model() const { return m_; }
  ParticleIndex get_particle_index() const { return pi_; }
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_PARTICLE_H */
