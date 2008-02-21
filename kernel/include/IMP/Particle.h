/**
 *  \file Particle.h     \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PARTICLE_H
#define __IMP_PARTICLE_H

#include <map>

#include "IMP_config.h"
#include "base_types.h"
#include "Model.h"
#include "internal/Object.h"
#include "utility.h"
#include "Key.h"
#include "internal/AttributeTable.h"
#include "DerivativeAccumulator.h"
#include "internal/ObjectPointer.h"

namespace IMP
{


class Model;

//! Class to handle individual model particles.
/** This class contains particle methods and indexes to particle attributes.
    Particles can be deactivated so that they no longer play a role in model
    optimization. Removing particles and their attributes would cause
    problems in the way attribute values are indexed and should not be done.
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
     This is for use in python mostly. C++ iterator will be added
     at some point.

     I would like to have a type-agnostic way of calling this 
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

protected:

  template <class T>
  T get_value_t(Index<T> k) const {
    IMP_check(get_is_active(), "get_value called on inactive Particle",
              InactiveParticleException());
    return model_->get_model_data()->get_value(k);
  }

  template <class T>
  void set_value_t(Index<T> k, const T&v) {
    IMP_check(get_is_active(), "set_value called on inactive Particle",
              InactiveParticleException());
    model_->get_model_data()->set_value(k, v);
  }

  // Set pointer to model particle data.
  /* This is called by the Model after the particle is added.
      \param[in] md Pointer to a ModelData object.
   */
  void set_model(Model *md, ParticleIndex pi);

  // all of the particle data
  internal::ObjectPointer<Model, false> model_;

  // true if particle is active
  bool is_active_;

  // float attributes associated with the particle
  internal::AttributeTable<Float> float_indexes_;
  // int attributes associated with the particle
  internal::AttributeTable<Int>  int_indexes_;
  // string attributes associated with the particle
  internal::AttributeTable<String>  string_indexes_;

  ParticleIndex pi_;
};


IMP_OUTPUT_OPERATOR(Particle)



inline bool Particle::has_attribute(FloatKey name) const
{
  return float_indexes_.contains(name);
}



inline Float Particle::get_value(FloatKey name) const
{
  return get_value_t(float_indexes_.get_value(name));
}

inline Float Particle::get_derivative(FloatKey name) const
{
  return model_->get_model_data()->get_deriv(float_indexes_.get_value(name));
}

inline void Particle::set_value(FloatKey name, Float value)
{
  set_value_t(float_indexes_.get_value(name), value);
}

inline bool Particle::get_is_optimized(FloatKey name) const
{
  return model_->get_model_data()
    ->get_is_optimized(float_indexes_.get_value(name));
}

inline void Particle::set_is_optimized(FloatKey name, bool tf)
{
  model_->get_model_data()
    ->set_is_optimized(float_indexes_.get_value(name), tf);
}

inline void Particle::add_to_derivative(FloatKey name, Float value,
                                        const DerivativeAccumulator &da)
{
  return model_->get_model_data()->add_to_deriv(float_indexes_.get_value(name),
                                                da(value));
}

inline bool Particle::has_attribute(IntKey name) const
{
  return int_indexes_.contains(name);
}



inline Int Particle::get_value(IntKey name) const
{
  return get_value_t(int_indexes_.get_value(name));
}


inline void Particle::set_value(IntKey name, Int value)
{
  return set_value_t(int_indexes_.get_value(name), value);
}

inline bool Particle::has_attribute(StringKey name) const
{
  return string_indexes_.contains(name);
}



inline String Particle::get_value(StringKey name) const
{
  return get_value_t(string_indexes_.get_value(name));
}

inline void Particle::set_value(StringKey name, String value)
{
  return set_value_t(string_indexes_.get_value(name), value);
}

} // namespace IMP

#endif  /* __IMP_PARTICLE_H */
