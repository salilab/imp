/**
 *  \file Particle.h     \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_PARTICLE_H
#define IMP_PARTICLE_H

#include "config.h"
#include "base_types.h"
#include "Object.h"
#include "internal/particle.h"
#include "utility.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include "DerivativeAccumulator.h"
#include "Pointer.h"
#include "VectorOfRefCounted.h"
#include <utility>

// should use this once we move to a new enough boost (1.35)
//#include <boost/intrusive/list.hpp>

#include <limits>
#include <list>

#define IMP_CHECK_ACTIVE                                                \
  IMP_check(get_is_active(), "Do not touch inactive particles "         \
            << get_name(), InactiveParticleException)
#define IMP_CHECK_MUTABLE IMP_IF_CHECK(CHEAP) {assert_values_mutable();}
#define IMP_CHECK_VALID_DERIVATIVES IMP_IF_CHECK(CHEAP) \
  {assert_valid_derivatives();}
#define IMP_PARTICLE_ATTRIBUTE_TYPE(UCName, lcname, Value, table,       \
                                    add_action, remove_action)          \
  void add_attribute(UCName##Key name, Value initial_value){            \
    IMP_CHECK_ACTIVE;                                                   \
    IMP_CHECK_MUTABLE;                                                  \
    IMP_check(name != UCName##Key(), "Cannot use attributes without "   \
              << "naming them.", ValueException);                       \
    IMP_check(!has_attribute(name),                                     \
              "Cannot add attribute " << name << " to particle "        \
              << get_name() << " twice.",                               \
              InvalidStateException);                                   \
    IMP_check(UCName##Table::Traits::get_is_valid(initial_value),       \
              "Initial value is not valid when adding attribute"        \
                               << name << " to particle " << get_name(),\
              ValueException);                                          \
    on_changed();                                                       \
    add_action;                                                         \
    table.add(name.get_index(), initial_value);                         \
  }                                                                     \
  void remove_attribute(UCName##Key name) {                             \
    IMP_CHECK_ACTIVE;                                                   \
    IMP_check(name != UCName##Key(), "Cannot use attributes without "   \
              << "naming them.", ValueException);                       \
    on_changed();                                                       \
    remove_action;                                                      \
    IMP_check(has_attribute(name),                                      \
              "Cannot remove attribute " << name << " from particle "   \
              << get_name() << " as it is not there.",                  \
              InvalidStateException);                                   \
    table.remove(name.get_index());                                     \
  }                                                                     \
  bool has_attribute(UCName##Key name) const{                           \
    IMP_check(name != UCName##Key(), "Cannot use attributes without "   \
              << "naming them.", ValueException);                       \
    IMP_CHECK_ACTIVE;                                                   \
    if (!table.fits(name.get_index())) return false;                    \
    else {                                                              \
      return UCName##Table::Traits::get_is_valid(                       \
                                          table.get(name.get_index())); \
    }                                                                   \
  }                                                                     \
  Value get_value(UCName##Key name) const {                             \
    IMP_CHECK_ACTIVE;                                                   \
    IMP_check(name != UCName##Key(), "Cannot use attributes without "   \
              << "naming them.", ValueException);                       \
    IMP_check(has_attribute(name),                                      \
              "Cannot get value " << name << " from particle "          \
              << get_name() << " as it is not there.",                  \
              InvalidStateException);                                   \
    return table.get(name.get_index());                                 \
  }                                                                     \
  void set_value(UCName##Key name, Value value) {                       \
    IMP_check(name != UCName##Key(), "Cannot use attributes without "   \
              << "naming them.", ValueException);                       \
    IMP_check(UCName##Table::Traits::get_is_valid(value),               \
              "Cannot set value of " << name << " to " << value         \
              << " on particle " << get_name(),                         \
              ValueException);                                          \
    IMP_CHECK_ACTIVE;                                                   \
    IMP_CHECK_MUTABLE;                                                  \
    IMP_check(has_attribute(name),                                      \
              "Cannot set value " << name << " from particle "          \
              << get_name() << " as it is not there.",                  \
              InvalidStateException);                                   \
    on_changed();                                                       \
    table.set(name.get_index(), value);                                 \
  }                                                                     \
  IMP_SWITCH_DOXYGEN(class UCName##KeyIterator,                         \
         typedef UCName##IteratorTraits::Iterator UCName##KeyIterator); \
  UCName##KeyIterator lcname##_keys_begin() const {                     \
    return UCName##IteratorTraits::create_iterator(this, 0,             \
                                                   table.get_length()); \
  }                                                                     \
  UCName##KeyIterator lcname##_keys_end() const {                       \
    return UCName##IteratorTraits::create_iterator(this,                \
                                                   table.get_length(),  \
                                                   table.get_length()); \
  }                                                                     \
  UCName##Keys get_##lcname##_attributes() const {                      \
    return UCName##Keys(lcname##_keys_begin(),                          \
                        lcname##_keys_end());                           \
  }



IMP_BEGIN_NAMESPACE

class Model;
class Changed;
class SaveOptimizeds;

//! Class to handle individual model particles.
/**

   \note Direct manipuation of particles is considered advanced
   and Particles should only be manipulated through
   \ref decorators "Decorators".

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

   Then use it to maniputate the attribute.
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
   have results is undefined. It will throw an exception if checks are
   or possibly just crash if they are not. Likewise an attempt to touch
   an inactive particle is also undefined (and will throw an exception if
   checks are enabled).
*/
class IMPEXPORT Particle : public Object
{
 private:
  // doxygen produces funny docs for these things
#ifndef IMP_DOXYGEN
  friend class Model;
  friend class Changed;
  friend class SaveOptimizeds;
  //typedef internal::ObjectContainer<Particle, unsigned int> Storage;
  typedef std::list<Particle*> Storage;

  void zero_derivatives();

  void assert_values_mutable() const;

  void assert_can_change_optimization() const;

  void assert_can_change_derivatives() const;

  void assert_valid_derivatives() const;


 // begin incremental
  void on_changed() {
    dirty_=true;
  }

  void set_is_not_changed() {
    if (dirty_) {
      shadow_->floats_= floats_;
      shadow_->strings_= strings_;
      shadow_->ints_= ints_;
      shadow_->optimizeds_= optimizeds_;
      shadow_->particles_.clear();
      for (ParticleKeyIterator it= particle_keys_begin();
           it != particle_keys_end(); ++it) {
        shadow_->particles_.add(it->get_index(),
                                get_value(*it)->shadow_);
      }
    }
    dirty_=false;
  }

  void setup_incremental();

  void teardown_incremental();

  // don't add the particle to the model, used for incremental
  Particle();

  void accumulate_derivatives_from_shadow();
  void move_derivatives_to_shadow();
  // end incremental

  typedef internal::ArrayStorage<internal::FloatAttributeTableTraits>
    FloatTable;
  typedef internal::ArrayStorage<internal::BoolAttributeTableTraits>
    OptimizedTable;
  typedef internal::ArrayStorage<internal::IntAttributeTableTraits>
    IntTable;
  typedef internal::ArrayStorage<internal::StringAttributeTableTraits>
    StringTable;
  typedef internal::RefCountedStorage<internal::ParticlesAttributeTableTraits>
    ParticleTable;
  typedef internal::RefCountedStorage<internal::ObjectsAttributeTableTraits>
    ObjectTable;
  typedef internal::ArrayStorage<internal::DoubleAttributeTableTraits>
    DerivativeTable;

  typedef internal::ParticleKeyIterator<FloatKey, Particle,
    internal::IsAttribute<FloatKey, Particle> > FloatIteratorTraits;
  typedef internal::ParticleKeyIterator<IntKey, Particle,
    internal::IsAttribute<IntKey, Particle> > IntIteratorTraits;
  typedef internal::ParticleKeyIterator<StringKey, Particle,
    internal::IsAttribute<StringKey, Particle> > StringIteratorTraits;
  typedef internal::ParticleKeyIterator<ParticleKey, Particle,
    internal::IsAttribute<ParticleKey, Particle> > ParticleIteratorTraits;
  typedef internal::ParticleKeyIterator<ObjectKey, Particle,
    internal::IsAttribute<ObjectKey, Particle> > ObjectIteratorTraits;


  typedef internal::ParticleKeyIterator<FloatKey, Particle,
    internal::IsOptimized<FloatKey, Particle> > OptimizedIteratorTraits;

 private:
  WeakPointer<Model> model_;

  FloatTable floats_;
  DerivativeTable derivatives_;
  OptimizedTable optimizeds_;
  IntTable ints_;
  StringTable  strings_;
  ParticleTable particles_;
  ObjectTable objects_;

  Storage::iterator iterator_;

  // incremental updates
  bool dirty_;
  // manually ref counted since Pointer requires the full definition
  Particle* shadow_;
#endif

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Particle);
 public:

  //! Construct a particle and add it to the Model
  Particle(Model *m, std::string name="P%1%");

  /** Get pointer to Model containing this particle.
      \throw InvalidStateException if now Model contains this particle.
  */
  Model* get_model() const {
    return model_;
  }

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
  IMP_PARTICLE_ATTRIBUTE_TYPE(Float, float, Float, floats_,
                              { derivatives_.add(name.get_index(), 0);},
                              {if (optimizeds_.fits(name.get_index())) {
                                  optimizeds_.remove(name.get_index());
                                }
                                derivatives_.remove(name.get_index());});

#ifdef IMP_DOXYGEN
  class OptimizedKeyIterator;
#else
  typedef OptimizedIteratorTraits::Iterator OptimizedKeyIterator;
#endif
  OptimizedKeyIterator optimized_keys_begin() const {
    return OptimizedIteratorTraits::create_iterator(this, 0,
                                                    floats_.get_length());
  }
  OptimizedKeyIterator optimized_keys_end() const {
    return OptimizedIteratorTraits::create_iterator(this,
                                                    floats_.get_length(),
                                                    floats_.get_length());
  }

  IMP_PARTICLE_ATTRIBUTE_TYPE(Int, int, Int,ints_,,);
  IMP_PARTICLE_ATTRIBUTE_TYPE(String, string, String,strings_,,)
  IMP_PARTICLE_ATTRIBUTE_TYPE(Particle, particle, Particle*,particles_,,)
  IMP_PARTICLE_ATTRIBUTE_TYPE(Object, object, Object*,objects_,,);
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
  void add_attribute(FloatKey name, const Float initial_value, bool optimized){
    add_attribute(name, initial_value);
    if (optimized) set_is_optimized(name, optimized);
  }

  void add_to_derivative(FloatKey key, Float value,
                         const DerivativeAccumulator &da);

  void set_is_optimized(FloatKey k, bool tf);

  bool get_is_optimized(FloatKey k) const;

  Float get_derivative(FloatKey name) const;

  /** @} */

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
  */
  bool get_is_active() const {
    IMP_IF_CHECK(EXPENSIVE) {
      IMP_assert(get_is_valid(), "Particle has been previously freed.");
    }
    return model_;
  }

  VersionInfo get_version_info() const {
    return IMP::get_module_version_info();
  }

  //! Show the particle
  /** \param[in] out Stream to write particle description to.

      All the attributes are shown. In addition, the deriviatives of the
      optimized attributes are printed.
  */
  void show(std::ostream& out = std::cout) const;

   /** \name Incremental Updates

      Control whether incremental updates are being used. See
      the \ref incremental "incremental updates" page for a more
      detailed description.
      @{
  */
  //! Return true if this particle has been changed since the last evaluate call
  bool get_is_changed() const {
    return dirty_;
  }
  /** \brief Return the shadow particle having attribute values from the last
      evaluation
  */
  Particle *get_prechange_particle() const {
    return shadow_;
  }
  /** @} */
};


IMP_OUTPUT_OPERATOR(Particle)

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_CHECK_ACTIVE;
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  IMP_CHECK_VALID_DERIVATIVES;
  return derivatives_.get(name.get_index());
}


inline bool Particle::get_is_optimized(FloatKey name) const
{
  IMP_CHECK_ACTIVE;
  /*IMP_check(floats_.contains(name), "get_is_optimized called "
            << "with invalid attribute " << name,
            IndexException);*/
  if (!optimizeds_.fits(name.get_index())) return false;
  else return optimizeds_.get(name.get_index());
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  IMP_CHECK_ACTIVE;
  IMP_check(has_attribute(name), "set_is_optimized called "
            << "with invalid attribute" << name,
            IndexException);
  IMP_IF_CHECK(CHEAP) {assert_can_change_optimization();}

  if (tf) {
    optimizeds_.add(name.get_index(), true);
  } else {
    optimizeds_.remove(name.get_index());
  }
}

inline void Particle::add_to_derivative(FloatKey name, Float value,
                                        const DerivativeAccumulator &da)
{
  IMP_CHECK_ACTIVE;
  IMP_assert(!is_nan(value), "Can't add NaN to derivative in particle "
             << *this);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  IMP_IF_CHECK(CHEAP) { assert_can_change_derivatives();}
  IMP_assert(name.get_index() < derivatives_.get_length(),
             "Something is wrong with derivative table.");
  derivatives_.set(name.get_index(),
                   derivatives_.get(name.get_index())+ da(value));
}


//! A class to store a pair of particles.
/** \note These do not due ref counting currently. SWIG prevents
    use of Pointer<Particle> as the storage type without some
    gynmastics.

    \note ParticlePair objects are ordered.
*/
class ParticlePair: public NullDefault,
                    public Comparable {
public:
  typedef ParticlePair This;
  Particle *first, *second;
  ParticlePair(): first(NULL), second(NULL){}
  ParticlePair(Particle *a, Particle *b):
    first(a), second(b) {}
  IMP_COMPARISONS_2(first, second)
  Particle * operator[](unsigned int i) const {
    switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    default:
      throw IndexException("Invalid member of pair");
    }
  }
  Particle *& operator[](unsigned int i) {
    switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    default:
      throw IndexException("Invalid member of pair");
    }
  }

  std::string get_name() const {
    return first->get_name() + " and " +second->get_name();
  }
  IMP_SHOWABLE_INLINE({
      out << "(";
      if (first) out << first->get_name();
      else out << "NULL";
      out << ", ";
      if (second) out << second->get_name();
      else out << "NULL";
      out << ")";
    });
};


#if !defined(IMP_DOXYGEN)
struct RefCountParticlePair {
  template <class O>
  static void ref(O o) {
    internal::ref(o[0]);
    internal::ref(o[1]);
  }
  template <class O>
  static void unref(O o) {
    internal::unref(o[0]);
    internal::unref(o[1]);
  }
};
#endif

typedef VectorOfRefCounted<ParticlePair, RefCountParticlePair> ParticlePairs;
typedef std::vector<ParticlePair> ParticlePairsTemp;

IMP_OUTPUT_OPERATOR(ParticlePair);


//! Store three particles
/**     \note ParticleTriplet objects are ordered.

 */
class ParticleTriplet: public NullDefault,
                       public Comparable {
  bool is_default() const {return false;}
public:
  typedef ParticleTriplet This;
  Particle *first, *second, *third;
  ParticleTriplet(): first(NULL), second(NULL), third(NULL){}
  ParticleTriplet(Particle *a, Particle *b, Particle *c):
    first(a), second(b), third(c) {}
  IMP_COMPARISONS_3(first, second, third)
  Particle *operator[](unsigned int i) const {
    switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    case 2:
      return third;
    default:
      throw IndexException("Invalid member of triplet");
    };
  }
  Particle *&operator[](unsigned int i) {
    switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    case 2:
      return third;
    default:
      throw IndexException("Invalid member of triplet");
    };
  }
  IMP_SHOWABLE_INLINE({
      out << "(";
      if (first) out << first->get_name();
      else out << "NULL";
      out << ", ";
      if (second) out << second->get_name();
      else out << "NULL";
      out << ", ";
      if (third) out << third->get_name();
      else out << "NULL";
      out << ")";
    })

};

#if!defined(IMP_DOXYGEN)
struct RefCountParticleTriplet {
  template <class O>
  static void ref(O o) {
    internal::ref(o[0]);
    internal::ref(o[1]);
    internal::ref(o[2]);
  }
  template <class O>
  static void unref(O o) {
    internal::unref(o[0]);
    internal::unref(o[1]);
    internal::unref(o[2]);
  }
};
#endif


typedef VectorOfRefCounted<ParticleTriplet,
                           RefCountParticleTriplet> ParticleTriplets;

IMP_OUTPUT_OPERATOR(ParticleTriplet);


//! A class which is used for representing collections of particles
typedef VectorOfRefCounted<Particle*> Particles;


//! A type to use when returning sets of particles so as to avoid refcounting
/** Always store using Particles instead.
 */
typedef std::vector<Particle*> ParticlesTemp;


#if !defined(SWIG) && !defined(IMP_DOXYGEN)
inline std::ostream &operator<<(std::ostream &out, const Particles &ps) {
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (ps[i]) {
      out << ps[i]->get_name() << " ";
    } else {
      out << "NULL ";
    }
  }
  return out;
}
#endif

IMP_END_NAMESPACE

#undef IMP_CHECK_ACTIVE
#undef IMP_CHECK_MUTABLE
#undef IMP_CHECK_VALID_DERIVATIVES

#include "Model.h"

#endif  /* IMP_PARTICLE_H */
