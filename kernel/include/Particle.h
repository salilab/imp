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
    To merely prevent a particle from moving during
    optimization, mark all of its attributes as being non-optimizable
    (set_is_optimized method).

    A particle may only belong to one model.

    \note In general, Particles should only be used through
    \ref decorators "Decorators" as these provide a nice and more reliable
    interface.
 */
class IMPEXPORT Particle : public RefCountedObject
{
private:
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
 typedef internal::AttributeTable<internal::FloatAttributeTableTraits>
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
#ifndef IMP_NO_DEPRECATED
  unsigned int index_;
#endif

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

public:

  //! Construct a particle and add it to the Model
  Particle(Model *m);

  Particle();
  ~Particle();

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
  //! Add a Float attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
      \param[in] is_optimized Whether the attribute's value should be
                              optimizable.
   */
  void add_attribute(FloatKey name, const Float value,
                     const bool is_optimized = false);

  /** Remove a Float attribute from this particle.
      \param[in] name Name of the attribute being added.
   */
  void remove_attribute(FloatKey name);


  /** Does particle have a Float attribute with the given name.
      \param[in] name Name of the attribute being checked.
      \return true if Float attribute exists in this particle.
   */
  bool has_attribute(FloatKey name) const;

  /** Get the specified Float for this particle.
      \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return the value of this attribute.
   */
  Float get_value(FloatKey name) const;

  /** Set the specified Float for this particle.
      \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(FloatKey name, Float value);

  //! Add to the derivative of a specified float.
  /** \param[in] name Name of the attribute being modified.
      \param[in] value Value being added.
      \param[in] da The DerivativeAccumulator to scale the value.
      \exception std::out_of_range attribute does not exist.
   */
  void add_to_derivative(FloatKey name, Float value,
                         const DerivativeAccumulator &da);

  /** Set whether this float attribute is optimized.*/
  void set_is_optimized(FloatKey k, bool tf);

  /** Return whether this float attribute is optimized.*/
  bool get_is_optimized(FloatKey k) const;

  /** Get the derivative of a specified Float.
      \param[in] name Name of the attribute being modified.
      \exception std::out_of_range attribute does not exist.
   */
  Float get_derivative(FloatKey name) const;

  //! An iterator through the keys of the float attributes of this particle
  typedef FloatTable::AttributeKeyIterator
    FloatKeyIterator;
  //! Iterate through the keys of float attributes of the particle
  FloatKeyIterator float_keys_begin() const {
    return floats_.attribute_keys_begin();
  }
  FloatKeyIterator float_keys_end() const {
    return floats_.attribute_keys_end();
  }

  //! An iterator through the keys of the derivative attributes of this particle
  typedef OptimizedTable::AttributeKeyIterator
    OptimizedKeyIterator;
  //! Iterate through the keys of float attributes of the particle
  OptimizedKeyIterator optimized_keys_begin() const {
    return optimizeds_.attribute_keys_begin();
  }
  OptimizedKeyIterator optimized_keys_end() const {
    return optimizeds_.attribute_keys_end();
  }
  /*@}*/


  /** @name Int Attributes*/
  /*@{*/
  //! Add an Int attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(IntKey name, const Int value);

  /** Remove a Int attribute from this particle.
      \param[in] name Name of the attribute being added.
   */
  void remove_attribute(IntKey name);

  /** Does particle have an Int attribute with the given name.
      \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(IntKey name) const;

  /** Get the specified Int for this particle.
      \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  Int get_value(IntKey name) const;

  /**  Set the specified Int for this particle.
       \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(IntKey name, Int value);

  //! An iterator through the keys of the int attributes of this particle
  typedef IntTable::AttributeKeyIterator IntKeyIterator;
  //! Iterate through the keys of int attributes of the particle
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

  /**  Remove a String attribute from this particle.
       \param[in] name Name of the attribute being added.
   */
  void remove_attribute(StringKey name);


  /** Does particle have a String attribute with the given name.
      \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(StringKey name) const;

  /** Get the specified String for this particle.
      \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  String get_value(StringKey name) const;

  /** Set the specified String for this particle.
      \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(StringKey name, String value);

  //! An iterator through the keys of the string attributes of this particle
  typedef StringTable::AttributeKeyIterator StringKeyIterator;
  //! Iterate through the keys of string attributes of the particle
  StringKeyIterator string_keys_begin() const {
    return strings_.attribute_keys_begin();
  }
  StringKeyIterator string_keys_end() const {
    return strings_.attribute_keys_end();
  }
  /*@}*/


  /** @name Particle Attributes*/
  /*@{*/
  /** Add a Particle attribute to this particle.
      \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(ParticleKey name, Particle* value);

  /** Remove a Particle attribute from this particle.
      \param[in] name Name of the attribute being added.
   */
  void remove_attribute(ParticleKey name);

  /** Does particle have a Particle attribute with the given name.
      \param[in] name Name of the attribute being checked.
      \return true if Particle attribute exists in this particle.
   */
  bool has_attribute(ParticleKey name) const;

  /** Get the specified Particle for this particle.
      \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  Particle* get_value(ParticleKey name) const;

  /** Set the specified Particle for this particle.
      \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(ParticleKey name, Particle* value);

  //! An iterator through the keys of the Particle attributes of this particle
  typedef ParticleTable::AttributeKeyIterator ParticleKeyIterator;
  //! Iterate through the keys of Particle attributes of the particle
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
  //! Return a vector containing all the FloatKeys for the Particle
  std::vector<FloatKey> get_float_attributes() const {
    return floats_.get_keys();
  }

  //! See get_float_attributes
  std::vector<IntKey> get_int_attributes() const {
    return ints_.get_keys();
  }

  //! See get_float_attributes
  std::vector<StringKey> get_string_attributes() const {
    return strings_.get_keys();
  }

  //! See get_particle_attributes
  std::vector<ParticleKey> get_particle_attributes() const {
    return particles_.get_keys();
  }
  /*@}*/

  //! Return the name of the particle
  std::string get_name() const {
    return name_;
  }
#ifndef IMP_NO_DEPRECATED
  // for backwards compatibility, do not use
  unsigned int get_index() const {
    return index_;
  }
#endif

  //! Set the name of the particle
  void set_name(std::string name) {
    name_=name;
  }
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
  return floats_.get_value(name);
}

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
  return derivatives_.get_value(name);
}

inline void Particle::set_value(FloatKey name, Float value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException);
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
class ParticlePair {
  bool is_default() const {return false;}
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
class ParticleTriplet {
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
