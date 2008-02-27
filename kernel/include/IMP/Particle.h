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

struct IMPDLLEXPORT FloatData
{
  bool is_optimized;
  Float value;
  Float derivative;
  FloatData(Float v, bool opt): is_optimized(opt), value(v), derivative(0){}
  FloatData():is_optimized(false), value(std::numeric_limits<Float>::max()),
              derivative(std::numeric_limits<Float>::max()){}
};

inline std::ostream &operator<<(std::ostream &out, const FloatData &d)
{
  out << d.value << ": " << d.derivative;
  if (d.is_optimized) out << "(opt)";
  return out;
}

} // namespace internal

#endif
class Model;

//! Class to handle individual model particles.
/** This class contains particle methods and indexes to particle attributes.
    Particles cannot be deleted once they are added to a model, but they can
    be deactivated (with their set_is_active method) after which they play no
    role in the scoring (it is illegal to try to evaluate a restraint on an
    inactive particle). To merely prevent a particle from moving during
    optimization, mark all of its attributes as being non-optimizable
    (set_is_optimized method).
    \ingroup kernel
 */
class IMPDLLEXPORT Particle : public internal::Object
{
  friend class Model;
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
    return float_indexes_.get_keys();
  }

  //! See get_float_attributes
  std::vector<IntKey> get_int_attributes() const {
    return int_indexes_.get_keys();
  }

  //! See get_float_attributes
  std::vector<StringKey> get_string_attributes() const {
    return string_indexes_.get_keys();
  }

  //! An iterator through the keys of the float attributes of this particle
  typedef internal::AttributeTable<Float, 
    internal::FloatData>::AttributeKeyIterator
    FloatKeyIterator;
  //! Iterate through the keys of float attributes of the particle
  FloatKeyIterator float_keys_begin() const {
    return float_indexes_.attribute_keys_begin();
  }
  FloatKeyIterator float_keys_end() const {
    return float_indexes_.attribute_keys_end();
  }

  //! An iterator through the keys of the int attributes of this particle
  typedef internal::AttributeTable<Int, Int>
    ::AttributeKeyIterator IntKeyIterator;
  //! Iterate through the keys of int attributes of the particle
  IntKeyIterator int_keys_begin() const {
    return int_indexes_.attribute_keys_begin();
  }
  IntKeyIterator int_keys_end() const {
    return int_indexes_.attribute_keys_end();
  }

  //! An iterator through the keys of the string attributes of this particle
  typedef internal::AttributeTable<String, String>
    ::AttributeKeyIterator StringKeyIterator;
  //! Iterate through the keys of string attributes of the particle
  StringKeyIterator string_keys_begin() const {
    return string_indexes_.attribute_keys_begin();
  }
  StringKeyIterator string_keys_end() const {
    return string_indexes_.attribute_keys_end();
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
  internal::AttributeTable<Float, internal::FloatData> float_indexes_;
  // int attributes associated with the particle
  internal::AttributeTable<Int, Int>  int_indexes_;
  // string attributes associated with the particle
  internal::AttributeTable<String, String>  string_indexes_;

  ParticleIndex pi_;
};


IMP_OUTPUT_OPERATOR(Particle)



inline bool Particle::has_attribute(FloatKey name) const
{
  return float_indexes_.contains(name);
}



inline Float Particle::get_value(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return float_indexes_.get_value(name).value;
}

inline Float Particle::get_derivative(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return float_indexes_.get_value(name).derivative;
}

inline void Particle::set_value(FloatKey name, Float value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  IMP_assert(value==value, "Can't set value to NaN");
  float_indexes_.get_value(name).value= value;
}

inline bool Particle::get_is_optimized(FloatKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return float_indexes_.get_value(name).is_optimized;
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  float_indexes_.get_value(name).is_optimized=tf;
}

inline void Particle::add_to_derivative(FloatKey name, Float value,
                                        const DerivativeAccumulator &da)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  float_indexes_.get_value(name).derivative+= da(value);
}

inline bool Particle::has_attribute(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return int_indexes_.contains(name);
}



inline Int Particle::get_value(IntKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return int_indexes_.get_value(name);
}


inline void Particle::set_value(IntKey name, Int value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  int_indexes_.get_value(name)= value;
}

inline bool Particle::has_attribute(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return string_indexes_.contains(name);
}



inline String Particle::get_value(StringKey name) const
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  return string_indexes_.get_value(name);
}

inline void Particle::set_value(StringKey name, String value)
{
  IMP_check(get_is_active(), "Do not touch inactive particles",
            ValueException("Don't touch inactive particles"));
  string_indexes_.get_value(name)= value;
}

} // namespace IMP

#endif  /* __IMP_PARTICLE_H */
