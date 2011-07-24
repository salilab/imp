/**
 *  \file Particle.h     \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_PARTICLE_H
#define IMP_PARTICLE_H

#include "kernel_config.h"
#include "base_types.h"
#include "Object.h"
#include "utility.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include "DerivativeAccumulator.h"
#include "Pointer.h"
#include "container_base.h"
#include "particle_index.h"
#include <utility>


#define IMP_PARTICLE_ATTRIBUTE_TYPE_DECL(UCName, lcname, Value)         \
  void add_attribute(UCName##Key name, Value initial_value);            \
  void remove_attribute(UCName##Key name);                              \
  bool has_attribute(UCName##Key name) const;                           \
  Value get_value(UCName##Key name) const;                              \
  void set_value(UCName##Key name, Value value);                        \
  void add_cache_attribute(UCName##Key name, Value value);              \
  UCName##Keys get_##lcname##_keys() const


#define IMP_PARTICLE_ATTRIBUTE_TYPE_DEF(UCName, lcname, Value)          \
  inline void Particle::add_attribute(UCName##Key name, Value initial_value){ \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    get_model()->add_attribute(name, id_, initial_value);               \
  }                                                                     \
  inline void Particle::remove_attribute(UCName##Key name) {            \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    get_model()->remove_attribute(name, id_);                           \
  }                                                                     \
  inline bool Particle::has_attribute(UCName##Key name) const{          \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    return get_model()->get_has_attribute(name, id_);                   \
  }                                                                     \
  inline Value Particle::get_value(UCName##Key name) const {            \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    return get_model()->get_attribute(name, id_);                       \
  }                                                                     \
  inline void Particle::set_value(UCName##Key name, Value value) {      \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    get_model()->set_attribute(name, id_, value);                       \
  }                                                                     \
  inline UCName##Keys Particle::get_##lcname##_keys() const {           \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    return get_model()->UCName##AttributeTable::get_attribute_keys(id_); \
  }                                                                     \
  inline void Particle::add_cache_attribute(UCName##Key name,           \
                                            Value value) {              \
    IMP_USAGE_CHECK(get_is_active(), "Inactive particle used.");        \
    return get_model()->add_cache_attribute(name, id_, value);          \
  }



IMP_BEGIN_NAMESPACE

class Model;
class Changed;
class SaveOptimizeds;

//! Class to handle individual model particles.
/**

   \note Direct manipuation of particles is considered advanced
   and Particles should only be manipulated through
   IMP::Decorator derived classes.

   A IMP::Particle is a mapping between keys and values.

   Four possible types of values:
   - Float (float)
   - String (std::string or Python string)
   - Int (int)
   - Particle (A pointer to another IMP::Particle)

   To use an attribute you first create a key
   \verbatim
   f= IMP.FloatKey("MyAttribute")
   \endverbatim
   Creating a key is expensive and should not be done often.

   Then use it to manipulate the attribute.
   \verbatim
   p.add_attribute(f, initial_value, whether_attribute_is_optimized)
   p.set_attribute(f, new_value)
   p.remove_attribute(f)
   \endverbatim



   This class contains particle methods and indexes to particle attributes.
   To prevent a particle from being moved by the optimizer during
   optimization, mark all of its attributes as being non-optimizable
   (set_is_optimized method). Note that this only affects the optimizer,
   ScoreStates may still change the particle attributes.

   A particle may only belong to one model.

   Any attempt to access or change an attribute which the particle does not
   have results is undefined. It will throw an exception if checks are on
   or possibly just crash if they are not. Likewise an attempt to touch
   an inactive particle is also undefined (and will throw an exception if
   checks are enabled).
*/
class IMPEXPORT Particle : public Container
{
 private:
  // doxygen produces funny docs for these things
#ifndef IMP_DOXYGEN
  friend class Model;
#endif
  ParticleIndex id_;
  IMP_OBJECT(Particle);
 public:

  //! Construct a particle and add it to the Model
  Particle(Model *m, std::string name);

  //! Construct a particle and add it to the Model
  Particle(Model *m);


#ifdef IMP_DOXYGEN
  /** @name Attribute manipulation
      For each type of attribute and their corresponding key type,
      the Particle provides the following methods. The Type is
      the type of the attribute (Float, Int, Particle * etc.) and
      KeyType is the type of the key (FloatKey, IntKey, ParticleKey etc.).
      @{
  */
  void add_attribute(KeyType name, Type initial_value);
  void remove_attribute(KeyType name);
  bool has_attribute(KeyType name) const;
  Type get_value(KeyType name) const;
  /* @} */
#else

  IMP_PARTICLE_ATTRIBUTE_TYPE_DECL(Float, float, Float);
  IMP_PARTICLE_ATTRIBUTE_TYPE_DECL(Int, int, Int);
  IMP_PARTICLE_ATTRIBUTE_TYPE_DECL(String, string, String);
  IMP_PARTICLE_ATTRIBUTE_TYPE_DECL(Object, object, Object*);

#endif

 /** @name Float Attributes
      Float attributes can be optimized, meaning the optimizer is
      allowed to change their value in order to improve the score.
      As a result, there are a number of extra methods to manipulate
      them.

      All distances are assumed to be in angstroms
      and derivatives in kcal/mol angstrom. This is not enforced.
  */
  /*@{*/
  void add_attribute(FloatKey name, const Float initial_value, bool optimized);

  void add_to_derivative(FloatKey key, Float value,
                         const DerivativeAccumulator &da);

  void set_is_optimized(FloatKey k, bool tf);

  bool get_is_optimized(FloatKey k) const;

  Float get_derivative(FloatKey name) const ;
  /** @} */

  /** \name Particle attributes
      @{
  */
  void add_attribute(ParticleKey k, Particle *v);
  bool has_attribute(ParticleKey k);
  void set_value(ParticleKey k, Particle *v);
  Particle *get_value(ParticleKey k) const;
  void remove_attribute(ParticleKey k);
  ParticleKeys get_particle_keys() const;
  /** @} */

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
  */
  bool get_is_active() const {
    IMP_CHECK_OBJECT(this);
    return get_has_model();
  }

#if 0
#if !defined(IMP_DOXYGEN)&& !defined(SWIG)
  void *operator new(std::size_t sz, void*p);
  void operator delete(void *p);
  void *operator new(std::size_t sz);
#endif
#endif

  ParticleIndex get_index() const {
    return id_;
  }

#if !defined(IMP_DOXYGEN)
  ContainersTemp get_input_containers() const;
  bool get_contained_particles_changed() const;
  ParticlesTemp get_contained_particles() const;
  bool get_is_up_to_date() const { return true;}
  void clear_caches();
#endif
};

IMP_OUTPUT_OPERATOR(Particle);


IMP_END_NAMESPACE

#include "Model.h"

#endif  /* IMP_PARTICLE_H */
