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
#include "utility.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include "internal/kernel_version_info.h"
#include "internal/ObjectContainer.h"
#include "DerivativeAccumulator.h"
#include "Pointer.h"

// should use this once we move to a new enough boost (1.35)
//#include <boost/intrusive/list.hpp>

#include <limits>
#include <list>

IMP_BEGIN_NAMESPACE

class Model;

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
    have results in an IndexException. An attempt to touch a particle which
    is not part of a model results in an InactiveParticleException.

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
  //typedef internal::ObjectContainer<Particle, unsigned int> Storage;
  typedef std::list<Particle*> Storage;

  void zero_derivatives();

  /* This has to be declared here since boost 1.35 wants the full
     definition of Particle to be available when the Pointer
     is declared.
  */
  struct ParticleAttributeTableTraits
  {
    typedef Pointer<Particle> Value;
    typedef ParticleKey Key;
    static Value get_invalid() {
      return Value();
    }
    static bool get_is_valid(Value f) {
      return f!= Value();
    }
  };

 typedef internal::AttributeTable<internal::FloatAttributeTableTraits>
   FloatTable;
 typedef internal::AttributeTable<internal::DoubleAttributeTableTraits>
   DerivativeTable;
 typedef internal::AttributeTable<internal::BoolAttributeTableTraits>
   OptimizedTable;
 typedef internal::AttributeTable<internal::IntAttributeTableTraits>
   IntTable;
  typedef internal::AttributeTable<internal::StringAttributeTableTraits>
    StringTable;
  typedef internal::AttributeTable<ParticleAttributeTableTraits>
    ParticleTable;

  WeakPointer<Model> model_;

  std::string name_;

  // float attributes associated with the particle
  FloatTable floats_;
  // float attributes associated with the particle
  FloatTable derivatives_;
  // Whether a given float is optimized or not
  OptimizedTable optimizeds_;

  // int attributes associated with the particle
  IntTable ints_;
  // string attributes associated with the particle
  StringTable  strings_;
  // particle attributes associated with the particle
  ParticleTable particles_;

  Storage::iterator iterator_;
#endif

#if defined(SWIG)
 public:
#else
 private:
  IMP_NO_DOXYGEN(template <class T> friend void IMP::internal::unref(T*);)
  IMP_NO_DOXYGEN(friend class IMP::internal::UnRef<true>;)
#endif
  virtual ~Particle();

public:

  //! Construct a particle and add it to the Model
  Particle(Model *m);

  /** Get pointer to Model containing this particle.
      \throw InvalidStateException if now Model contains this particle.
   */
  Model* get_model() const {
    return model_;
  }

  /** @name Float Attributes
      Float attributes can be optimized, meaning the optimizer is
      allowed to change their value in order to improve the score.
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

  IMP_NO_DOXYGEN(typedef FloatTable::AttributeKeyIterator
                 FloatKeyIterator;)
  FloatKeyIterator float_keys_begin() const {
    return floats_.attribute_keys_begin();
  }
  FloatKeyIterator float_keys_end() const {
    return floats_.attribute_keys_end();
  }

  IMP_NO_DOXYGEN(typedef OptimizedTable::AttributeKeyIterator
                OptimizedKeyIterator;)
  OptimizedKeyIterator optimized_keys_begin() const {
    return optimizeds_.attribute_keys_begin();
  }
  OptimizedKeyIterator optimized_keys_end() const {
    return optimizeds_.attribute_keys_end();
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
  typedef IntTable::AttributeKeyIterator IntKeyIterator;
#endif

  IntKeyIterator int_keys_begin() const {
    return ints_.attribute_keys_begin();
  }
  IntKeyIterator int_keys_end() const {
    return ints_.attribute_keys_end();
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
  typedef StringTable::AttributeKeyIterator StringKeyIterator;
#endif
  StringKeyIterator string_keys_begin() const {
    return strings_.attribute_keys_begin();
  }
  StringKeyIterator string_keys_end() const {
    return strings_.attribute_keys_end();
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
  typedef ParticleTable::AttributeKeyIterator ParticleKeyIterator;
#endif
  ParticleKeyIterator particle_keys_begin() const {
    return particles_.attribute_keys_begin();
  }
  ParticleKeyIterator particle_keys_end() const {
    return particles_.attribute_keys_end();
  }
  /*@}*/

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
   */
  bool get_is_active() const {
    IMP_IF_CHECK(EXPENSIVE) {
      assert_is_valid();
    }
    return model_;
  }

  VersionInfo get_version_info() const {
    return internal::kernel_version_info;
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
    return floats_.get_keys();
  }
  IntKeys get_int_attributes() const {
    return ints_.get_keys();
  }
  StringKeys get_string_attributes() const {
    return strings_.get_keys();
  }
  ParticleKeys get_particle_attributes() const {
    return particles_.get_keys();
  }
  /*@}*/

  /** @name Names
      All particles have names to aid in debugging and inspection
      of the state of the system. These names are not necessarily unique
      and should not be used to store data or as keys into a table. If
      you need a unique key identifying a particle, add a new attribuite.
      @{
  */
  const std::string& get_name() const {
    return name_;
  }
  void set_name(std::string name) {
    name_=name;
  }
  /* @} */
};


IMP_OUTPUT_OPERATOR(Particle)



inline bool Particle::has_attribute(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return floats_.contains(name);
}



inline Float Particle::get_value(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  return floats_.get_value(name);
}

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  return derivatives_.get_value(name);
}

inline void Particle::set_value(FloatKey name, Float value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  floats_.set_value(name, value);
}

inline bool Particle::get_is_optimized(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_check(floats_.contains(name), "get_is_optimized called "
            << "with invalid attribute" << name,
            IndexException);
  return optimizeds_.contains(name);
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_check(floats_.contains(name), "set_is_optimized called "
            << "with invalid attribute" << name,
            IndexException);
  if (tf) {
    optimizeds_.insert_always(name, true);
  } else {
    optimizeds_.remove_always(name);
  }
}

inline void Particle::add_to_derivative(FloatKey name, Float value,
                                        const DerivativeAccumulator &da)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(!is_nan(value), "Can't add NaN to derivative in particle "
             << *this);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  derivatives_.set_value(name, derivatives_.get_value(name)+ da(value));
}

inline bool Particle::has_attribute(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return ints_.contains(name);
}



inline Int Particle::get_value(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(has_attribute(name), "Particle " << get_name()
             << " does not have attribute " << name);
  return ints_.get_value(name);
}


inline void Particle::set_value(IntKey name, Int value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  ints_.set_value(name, value);
}

inline bool Particle::has_attribute(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return strings_.contains(name);
}



inline String Particle::get_value(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return strings_.get_value(name);
}

inline void Particle::set_value(StringKey name, String value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  strings_.set_value(name, value);
}


inline bool Particle::has_attribute(ParticleKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return particles_.contains(name);
}



inline Particle* Particle::get_value(ParticleKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return particles_.get_value(name).get();
}


inline void Particle::set_value(ParticleKey name, Particle* value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  particles_.set_value(name, Pointer<Particle>(value));
}


void inline Particle::add_attribute(FloatKey name, const Float value,
                                    bool is_optimized)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(model_ ,
             "Particle must be added to Model before attributes are added");
  floats_.insert(name, value);
  derivatives_.insert(name, 0);
  if (is_optimized) {
    optimizeds_.insert(name, true);
  }
}

void inline Particle::remove_attribute(FloatKey name)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  floats_.remove(name);
  derivatives_.remove(name);
  optimizeds_.remove_always(name);
}


void inline Particle::add_attribute(IntKey name, const Int value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  ints_.insert(name, value);
}

void inline Particle::remove_attribute(IntKey name)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  ints_.remove(name);
}


void inline Particle::add_attribute(StringKey name, const String value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  strings_.insert(name, value);
}

void inline Particle::remove_attribute(StringKey name)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  strings_.remove(name);
}

void inline Particle::add_attribute(ParticleKey name, Particle* value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  particles_.insert(name,
                           Pointer<Particle>(value));
}


void inline Particle::remove_attribute(ParticleKey name)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
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

typedef std::vector<ParticlePair> ParticlePairs;


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

typedef std::vector<ParticleTriplet> ParticleTriplets;

IMP_OUTPUT_OPERATOR(ParticleTriplet);


IMP_END_NAMESPACE

#endif  /* IMP_PARTICLE_H */
