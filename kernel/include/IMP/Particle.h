/**
 *  \file Particle.h     \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PARTICLE_H
#define __IMP_PARTICLE_H

#include "IMP_config.h"
#include "base_types.h"
#include "Model.h"
#include "internal/Object.h"
#include "utility.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include "DerivativeAccumulator.h"
#include "internal/ObjectPointer.h"

#include <limits>

namespace IMP
{
#ifndef SWIG
namespace internal
{

template <class It>
void check_particles_active(It b, It e, std::string msg)
{
  for (It c= b; c != e; ++c) {
    IMP_check((*c)->get_is_active(), msg,
              InactiveParticleException(msg));
  }
}

} // namespace internal

#endif
class Model;

//! Class to handle individual model particles.
/** This class contains particle methods and indexes to particle attributes.
    To merely prevent a particle from moving during
    optimization, mark all of its attributes as being non-optimizable
    (set_is_optimized method).

    \ingroup kernel
 */
class IMPDLLEXPORT Particle : public internal::RefCountedObject
{
  friend class Model;

  /* This has to be declared here since boost 1.35 wants the full
     definition of Particle to be available when the ObjectPointer
     is declared.
  */
  struct ParticleAttributeTableTraits
  {
    typedef internal::ObjectPointer<Particle, true> Value;
    typedef KeyBase<Particle*> Key;
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

public:



  Particle();
  ~Particle();

  //! The index of this particle in the model
  ParticleIndex get_index() const {
    return pi_;
  }

  //! Get pointer to model particle data.
  /** \return all particle data in the model.
   */
  Model* get_model() const {
    return model_.get();
  }

  //! Add a Float attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
      \param[in] is_optimized Whether the attribute's value should be
                              optimizable.
   */
  void add_attribute(FloatKey name, const Float value,
                     const bool is_optimized = false);

  //! Remove a Float attribute from this particle.
  /** \param[in] name Name of the attribute being added.
   */
  void remove_attribute(FloatKey name);


  //! Does particle have a Float attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Float attribute exists in this particle.
   */
  bool has_attribute(FloatKey name) const;

  //! Get the specified Float for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return the value of this attribute.
   */
  Float get_value(FloatKey name) const;

  //! Set the specified Float for this particle.
  /** \param[in] name Name of the attribute being set.
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

  //! Set whether this float attribute is optimized
  void set_is_optimized(FloatKey k, bool tf);

  //! Return whether this float attribute is optimized
  bool get_is_optimized(FloatKey k) const;

  //! Get the derivative of a specified Float.
  /** \param[in] name Name of the attribute being modified.
      \exception std::out_of_range attribute does not exist.
   */
  Float get_derivative(FloatKey name) const;

  //! Add an Int attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(IntKey name, const Int value);

  //! Remove a Int attribute from this particle.
  /** \param[in] name Name of the attribute being added.
   */
  void remove_attribute(IntKey name);

  //! Does particle have an Int attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(IntKey name) const;

  //! Get the specified Int for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  Int get_value(IntKey name) const;

  //! Set the specified Int for this particle.
  /** \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(IntKey name, Int value);

  //! Add a String attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(StringKey name, const String value);

  //! Remove a String attribute from this particle.
  /** \param[in] name Name of the attribute being added.
   */
  void remove_attribute(StringKey name);


  //! Does particle have a String attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Int attribute exists in this particle.
   */
  bool has_attribute(StringKey name) const;

  //! Get the specified String for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  String get_value(StringKey name) const;

  //! Set the specified String for this particle.
  /** \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(StringKey name, String value);


  //! Add a Particle attribute to this particle.
  /** \param[in] name Name of the attribute being added.
      \param[in] value Initial value of the attribute.
   */
  void add_attribute(ParticleKey name, Particle* value);

  //! Remove a Particle attribute from this particle.
  /** \param[in] name Name of the attribute being added.
   */
  void remove_attribute(ParticleKey name);

  //! Does particle have a Particle attribute with the given name.
  /** \param[in] name Name of the attribute being checked.
      \return true if Particle attribute exists in this particle.
   */
  bool has_attribute(ParticleKey name) const;

  //! Get the specified Particle for this particle.
  /** \param[in] name Name of the attribute being retrieved.
      \exception std::out_of_range attribute does not exist.
      \return value of the attribute.
   */
  Particle* get_value(ParticleKey name) const;

  //! Set the specified Particle for this particle.
  /** \param[in] name Name of the attribute being set.
      \param[in] value Value of the attribute being set.
      \exception std::out_of_range attribute does not exist.
   */
  void set_value(ParticleKey name, Particle* value);


  //! Set whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \param[in] is_active If true, the particle is active.
   */
  void set_is_active(const bool is_active);

  //! Get whether the particle is active.
  /** Restraints referencing the particle are only evaluated for 'active'
      particles.
      \return true it the particle is active.
   */
  bool get_is_active() const {
    return is_active_;
  }

  //! Show the particle
  /** \param[in] out Stream to write particle description to.

      All the attributes are shown. In addition, the deriviatives of the
      optimized attributes are printed.
   */
  void show(std::ostream& out = std::cout) const;


  //! Return a vector containing all the FloatKeys for the Particle
  /**
     This is for use in python mostly. C++ users should use the iterators.

     \todo I would like to have a type-agnostic way of calling this 
     to be used to writing generic functions in python. The only
     ways I can think of doing this are to pass dummy arguments,
     which seems inelegant.
   */
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


  //! An iterator through the keys of the int attributes of this particle
  typedef IntTable::AttributeKeyIterator IntKeyIterator;
  //! Iterate through the keys of int attributes of the particle
  IntKeyIterator int_keys_begin() const {
    return ints_.attribute_keys_begin();
  }
  IntKeyIterator int_keys_end() const {
    return ints_.attribute_keys_end();
  }

  //! An iterator through the keys of the string attributes of this particle
  typedef StringTable::AttributeKeyIterator StringKeyIterator;
  //! Iterate through the keys of string attributes of the particle
  StringKeyIterator string_keys_begin() const {
    return strings_.attribute_keys_begin();
  }
  StringKeyIterator string_keys_end() const {
    return strings_.attribute_keys_end();
  }

  //! An iterator through the keys of the Particle attributes of this particle
  typedef ParticleTable::AttributeKeyIterator ParticleKeyIterator;
  //! Iterate through the keys of Particle attributes of the particle
  ParticleKeyIterator particle_keys_begin() const {
    return particles_.attribute_keys_begin();
  }
  ParticleKeyIterator particle_keys_end() const {
    return particles_.attribute_keys_end();
  }

protected:
  void zero_derivatives();

  // Set pointer to model particle data.
  void set_model(Model *md, ParticleIndex pi);

  // all of the particle data
  internal::ObjectPointer<Model, false> model_;

  // true if particle is active
  bool is_active_;

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

  ParticleIndex pi_;
};


IMP_OUTPUT_OPERATOR(Particle)



inline bool Particle::has_attribute(FloatKey name) const
{
  return floats_.contains(name);
}



inline Float Particle::get_value(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return floats_.get_value(name);
}

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return derivatives_.get_value(name);
}

inline void Particle::set_value(FloatKey name, Float value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  floats_.set_value(name, value);
}

inline bool Particle::get_is_optimized(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  IMP_check(floats_.contains(name), "get_is_optimized called "
            << "with invalid attribute" << name,
            IndexException("Invalid float attribute"));
  return optimizeds_.contains(name);
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  IMP_check(floats_.contains(name), "set_is_optimized called "
            << "with invalid attribute" << name,
            IndexException("Invalid float attribute"));
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
            InactiveParticleException());
  IMP_assert(value==value, "Can't add NaN to derivative in particle " << *this);
  derivatives_.set_value(name, derivatives_.get_value(name)+ da(value));
}

inline bool Particle::has_attribute(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return ints_.contains(name);
}



inline Int Particle::get_value(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return ints_.get_value(name);
}


inline void Particle::set_value(IntKey name, Int value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  ints_.set_value(name, value);
}

inline bool Particle::has_attribute(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return strings_.contains(name);
}



inline String Particle::get_value(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return strings_.get_value(name);
}

inline void Particle::set_value(StringKey name, String value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  strings_.set_value(name, value);
}


inline bool Particle::has_attribute(ParticleKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return particles_.contains(name);
}



inline Particle* Particle::get_value(ParticleKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  return particles_.get_value(name).get();
}


inline void Particle::set_value(ParticleKey name, Particle* value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            InactiveParticleException());
  particles_.set_value(name, internal::ObjectPointer<Particle, true>(value));
}


void inline Particle::add_attribute(FloatKey name, const Float value,
                                    bool is_optimized)
{
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
  floats_.remove(name);
  derivatives_.remove(name);
  optimizeds_.remove_always(name);
}


void inline Particle::add_attribute(IntKey name, const Int value)
{
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  ints_.insert(name, value);
}

void inline Particle::remove_attribute(IntKey name)
{
  ints_.remove(name);
}


void inline Particle::add_attribute(StringKey name, const String value)
{
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  strings_.insert(name, value);
}

void inline Particle::remove_attribute(StringKey name)
{
  strings_.remove(name);
}

void inline Particle::add_attribute(ParticleKey name, Particle* value)
{
  IMP_assert(model_,
             "Particle must be added to Model before attributes are added");
  particles_.insert(name,
                           internal::ObjectPointer<Particle, true>(value));
}


void inline Particle::remove_attribute(ParticleKey name)
{
  particles_.remove(name);
}

} // namespace IMP

#endif  /* __IMP_PARTICLE_H */
