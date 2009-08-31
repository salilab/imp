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

IMP_BEGIN_NAMESPACE

class Model;
class Changed;
class SaveOptimizeds;

//! Class to handle individual model particles.
/**
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

   \note In general, Particles should only be used through
   \ref decorators "Decorators" as these provide a nice and more reliable
   interface.
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
        shadow_->particles_.insert_always(*it,
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


  /* This has to be declared here since boost 1.35 wants the full
     definition of Particle to be available when the Pointer
     is declared.
  */
  struct ParticleAttributeTableTraits
  {
    typedef Pointer<Particle> Value;
    typedef Particle* PassValue;
    typedef ParticleKey Key;
    static Value get_invalid() {
      return Value();
    }
    static bool get_is_valid(const Value& f) {
      return f!= Value();
    }
  };

  typedef internal::AttributeTable<internal::FloatAttributeTableTraits,
    internal::ArrayStorage<internal::FloatAttributeTableTraits::Value> >
    FloatTable;
  typedef internal::AttributeTable<internal::BoolAttributeTableTraits,
    internal::ArrayStorage<internal::BoolAttributeTableTraits::Value> >
    OptimizedTable;
  typedef internal::AttributeTable<internal::IntAttributeTableTraits,
    internal::ArrayStorage<internal::IntAttributeTableTraits::Value> >
    IntTable;
  typedef internal::AttributeTable<internal::StringAttributeTableTraits,
    internal::ArrayStorage<internal::StringAttributeTableTraits::Value> >
    StringTable;
  typedef internal::AttributeTable<internal::ParticlesAttributeTableTraits,
    internal::ParticlesStorage<Particle*> >
    ParticleTable;
  typedef internal::ArrayStorage<double>  DerivativeTable;

  typedef internal::ParticleKeyIterator<FloatKey, Particle,
    internal::IsAttribute<FloatKey, Particle> > FloatIteratorTraits;
  typedef internal::ParticleKeyIterator<IntKey, Particle,
    internal::IsAttribute<IntKey, Particle> > IntIteratorTraits;
  typedef internal::ParticleKeyIterator<StringKey, Particle,
    internal::IsAttribute<StringKey, Particle> > StringIteratorTraits;
  typedef internal::ParticleKeyIterator<ParticleKey, Particle,
    internal::IsAttribute<ParticleKey, Particle> > ParticleIteratorTraits;


  typedef internal::ParticleKeyIterator<FloatKey, Particle,
    internal::IsOptimized<FloatKey, Particle> > OptimizedIteratorTraits;

 private:
  WeakPointer<Model> model_;

  // float attributes associated with the particle
  FloatTable floats_;
  // special case the derivatives since we never check for existence
  DerivativeTable derivatives_;
  // Whether a given float is optimized or not
  OptimizedTable optimizeds_;

  // int attributes associated with the particle
  IntTable ints_;
  // string attributes associated with the particle
  StringTable  strings_;
  // particle attributes associated with the particle
  ParticleTable particles_;

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

  /** @name Float Attributes
      Float attributes can be optimized, meaning the optimizer is
      allowed to change their value in order to improve the score.

      All distances are assumed to be in angstroms
      and derivatives in kcal/mol angstrom. This is not enforced.
  */
  /*@{*/
  /** \param[in] key The key identifying the float attribute.
      \param[in] value Initial value of the attribute.
      \param[in] is_optimized Whether the attribute's value can be
      changed by the optimizer.
  */
  void add_attribute(FloatKey key, const Float value,
                     const bool is_optimized = false);

  void remove_attribute(FloatKey name);


  bool has_attribute(FloatKey name) const;

  Float get_value(FloatKey name) const;

  void set_value(FloatKey name, Float value);

  //! Add to the derivative of a specified float.
  /** \param[in] key Key identifying the attribute.
      \param[in] value Amount to add to the derivative.
      \param[in] da The DerivativeAccumulator to scale the value.
  */
  void add_to_derivative(FloatKey key, Float value,
                         const DerivativeAccumulator &da);

  /** Set whether this float attribute will be changed by
      the optimizer. The attribute value is still allowed
      to change (for example in initialization or by ScoreStates.*/
  void set_is_optimized(FloatKey k, bool tf);

  bool get_is_optimized(FloatKey k) const;

  Float get_derivative(FloatKey name) const;

#ifdef IMP_DOXYGEN
  class FloatKeyIterator;
#else
  typedef FloatIteratorTraits::Iterator FloatKeyIterator;
#endif

  FloatKeyIterator float_keys_begin() const {
    return FloatIteratorTraits::create_iterator(this, 0, floats_.get_length());
  }
  FloatKeyIterator float_keys_end() const {
    return FloatIteratorTraits::create_iterator(this, floats_.get_length(),
                                                floats_.get_length());
  }

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
  /*@}*/


  /** @name Int Attributes*/
  /*@{*/
  /** \param[in] key The key identifying the attribute being added.
      \param[in] value Initial value of the attribute.
  */
  void add_attribute(IntKey key, const Int value);

  void remove_attribute(IntKey name);

  bool has_attribute(IntKey name) const;

  Int get_value(IntKey name) const;

  void set_value(IntKey name, Int value);

#ifdef IMP_DOXYGEN
  class IntKeyIterator;
#else
  typedef IntIteratorTraits::Iterator IntKeyIterator;
#endif

  IntKeyIterator int_keys_begin() const {
    return IntIteratorTraits::create_iterator(this, 0, ints_.get_length());
  }
  IntKeyIterator int_keys_end() const {
    return IntIteratorTraits::create_iterator(this, ints_.get_length(),
                                                ints_.get_length());
  }

  /*@}*/


  /** @name String Attributes*/
  /*@{*/

  /** Add a String attribute to this particle.
      \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
  */
  void add_attribute(StringKey name, const String value);

  void remove_attribute(StringKey name);

  bool has_attribute(StringKey name) const;

  String get_value(StringKey name) const;

  void set_value(StringKey name, String value);

#ifdef IMP_DOXYGEN
  class StringKeyIterator;
#else
  typedef StringIteratorTraits::Iterator StringKeyIterator;
#endif

  StringKeyIterator string_keys_begin() const {
    return StringIteratorTraits::create_iterator(this, 0,
                                                 strings_.get_length());
  }
  StringKeyIterator string_keys_end() const {
    return StringIteratorTraits::create_iterator(this,
                                                 strings_.get_length(),
                                                 strings_.get_length());
  }
  /*@}*/


  /** @name Particle Attributes
      Particle attributes store a pointer to another particle. They are
      useful for setting up graphs and hierarchies.
  */
  /*@{*/
  /** Add a Particle attribute to this particle.
      \param[in] key Name of the attribute being added.
      \param[in] value Initial value of the attribute.
  */
  void add_attribute(ParticleKey key, Particle* value);

  void remove_attribute(ParticleKey name);

  bool has_attribute(ParticleKey name) const;

  Particle* get_value(ParticleKey name) const;

  void set_value(ParticleKey name, Particle* value);

#ifdef IMP_DOXYGEN
  class ParticleKeyIterator;
#else
  typedef ParticleIteratorTraits::Iterator ParticleKeyIterator;
#endif

  ParticleKeyIterator particle_keys_begin() const {
    return ParticleIteratorTraits::create_iterator(this, 0,
                                                   particles_.get_length());
  }
  ParticleKeyIterator particle_keys_end() const {
    return ParticleIteratorTraits::create_iterator(this,
                                                   particles_.get_length(),
                                                   particles_.get_length());
  }
  /*@}*/

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

  /** @name Python accessors for the keys of all attributes
      These should only be used from python. Use the iterators in C++.
      \todo These should be move to the swig file and made %extends.
  */
  /*@{*/
  FloatKeys get_float_attributes() const {
    return FloatIteratorTraits::get_keys(this, floats_.get_length());
  }
  IntKeys get_int_attributes() const {
    return IntIteratorTraits::get_keys(this, ints_.get_length());
  }
  StringKeys get_string_attributes() const {
    return StringIteratorTraits::get_keys(this, strings_.get_length());
  }
  ParticleKeys get_particle_attributes() const {
    return ParticleIteratorTraits::get_keys(this, particles_.get_length());
  }
  /*@}*/

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



inline bool Particle::has_attribute(FloatKey name) const
{
  IMP_CHECK_ACTIVE;
  return floats_.contains(name);
}



inline Float Particle::get_value(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles "
            << get_name(),
            InactiveParticleException);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  return floats_.get_value(name);
}

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_CHECK_ACTIVE;
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  IMP_CHECK_VALID_DERIVATIVES;
  return derivatives_.get(name.get_index());
}

inline void Particle::set_value(FloatKey name, Float value)
{
  IMP_CHECK_ACTIVE;
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  IMP_CHECK_MUTABLE;
  on_changed();
  floats_.set_value(name, value);
}

inline bool Particle::get_is_optimized(FloatKey name) const
{
  IMP_CHECK_ACTIVE;
  /*IMP_check(floats_.contains(name), "get_is_optimized called "
            << "with invalid attribute " << name,
            IndexException);*/
  return optimizeds_.contains(name);
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  IMP_CHECK_ACTIVE;
  IMP_check(floats_.contains(name), "set_is_optimized called "
            << "with invalid attribute" << name,
            IndexException);
  IMP_IF_CHECK(CHEAP) {assert_can_change_optimization();}

  if (tf) {
    optimizeds_.insert_always(name, true);
  } else {
    optimizeds_.remove_always(name);
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

inline bool Particle::has_attribute(IntKey name) const
{
  IMP_CHECK_ACTIVE;
  return ints_.contains(name);
}



inline Int Particle::get_value(IntKey name) const
{
  IMP_CHECK_ACTIVE;
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  return ints_.get_value(name);
}


inline void Particle::set_value(IntKey name, Int value)
{
  IMP_CHECK_ACTIVE;
  IMP_CHECK_MUTABLE;
  on_changed();
  ints_.set_value(name, value);
}

inline bool Particle::has_attribute(StringKey name) const
{
  IMP_CHECK_ACTIVE;
  return strings_.contains(name);
}



inline String Particle::get_value(StringKey name) const
{
  IMP_CHECK_ACTIVE;
  return strings_.get_value(name);
}

inline void Particle::set_value(StringKey name, String value)
{
  IMP_CHECK_ACTIVE;
  IMP_CHECK_MUTABLE;
  on_changed();
  strings_.set_value(name, value);
}


inline bool Particle::has_attribute(ParticleKey name) const
{
  IMP_CHECK_ACTIVE;
  return particles_.contains(name);
}



inline Particle* Particle::get_value(ParticleKey name) const
{
  IMP_CHECK_ACTIVE;
  return particles_.get_value(name);
}


inline void Particle::set_value(ParticleKey name, Particle* value)
{
  IMP_CHECK_ACTIVE;
  IMP_CHECK_MUTABLE;
  on_changed();
  particles_.set_value(name, Pointer<Particle>(value));
}


void inline Particle::add_attribute(FloatKey name, const Float value,
                                    bool is_optimized)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  floats_.insert(name, value);
  derivatives_.add(name.get_index(), 0, 0);
  if (is_optimized) {
    optimizeds_.insert(name, true);
  }
}

void inline Particle::remove_attribute(FloatKey name)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  floats_.remove(name);
  //derivatives_.remove(name);
  optimizeds_.remove_always(name);
}


void inline Particle::add_attribute(IntKey name, const Int value)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  ints_.insert(name, value);
}

void inline Particle::remove_attribute(IntKey name)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  ints_.remove(name);
}


void inline Particle::add_attribute(StringKey name, const String value)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  strings_.insert(name, value);
}

void inline Particle::remove_attribute(StringKey name)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  strings_.remove(name);
}

void inline Particle::add_attribute(ParticleKey name, Particle* value)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  particles_.insert(name,value);
}


void inline Particle::remove_attribute(ParticleKey name)
{
  IMP_CHECK_ACTIVE;
  on_changed();
  particles_.remove(name);
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
  void show(std::ostream &out= std::cout) const {
    out << "(";
    if (first) out << first->get_name();
    else out << "NULL";
    out << ", ";
    if (second) out << second->get_name();
    else out << "NULL";
    out << ")";
  }
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
  void show(std::ostream &out= std::cout) const {
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
  }

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
