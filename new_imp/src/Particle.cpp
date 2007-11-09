/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"
#include "mystdexcept.h"


namespace IMP
{

namespace internal {

std::vector<AttributeKeyData> attribute_key_data;

// These really should go in the header file,
// but I can't throw exceptions from there

template <class T>
inline void AttributeTable<T>::insert(Key k, Value v)
{
  if (map_.size() <= k.get_index()) {
    map_.resize(k.get_index()+1);
  }
  IMP_assert(map_[k.get_index()]== Value(),
             "Trying to add attribute \"" << k.get_string()
             << "\" twice");
  map_[k.get_index()]= v;
}

template <class T>
inline const typename AttributeTable<T>::Value
    AttributeTable<T>::operator[](Key k) const
{
  IMP_check(k.get_index()< map_.size()
            && map_[k.get_index()] != Value(),
            "Attribute \"" << k.get_string()
            << "\" not found in table.",
            std::out_of_range(std::string("Invalid attribute \"")
                              + k.get_string() + "\" requested"));
  return map_[k.get_index()];
}

template <class T>
inline bool AttributeTable<T>::contains(Key k) const
{
  return map_.size() < k.get_index()
         && map_[k.get_index()] != Value();
}

template <class T>
inline std::ostream &AttributeTable<T>::show(std::ostream &out,
                                             const char *prefix) const
{
  for (unsigned int i=0; i< map_.size(); ++i) {
    if (map_[i] != Value()) {
      out << prefix << "\""
      << Key::get_string(i) << "\": " << map_[i] << std::endl;
    }
  }
  return out;
}

} /* namespace internal */


//! Constructor
Particle::Particle(): model_(NULL)
{
  IMP_LOG(VERBOSE, "create particle");
  is_active_ = true;
}


//! Destructor
Particle::~Particle()
{
  IMP_LOG(VERBOSE, "delete particle");
}


//! Get pointer to model particle data.
/** \return all particle data in the model.
 */
Model* Particle::get_model(void) const
{
  return model_;
}

//! Set pointer to model particle data.
/** This is called by the Model after the particle is added.
    \param[in] md Pointer to a ModelData object.
 */
void Particle::set_model(Model *md)
{
  model_=md;
}


//! Set whether the particle is active.
/** Restraints referencing the particle are only evaluated for 'active'
    particles.
    \param[in] is_active If true, the particle is active.
 */
void Particle::set_is_active(const bool is_active)
{
  is_active_ = is_active;

  // indicate to restraints that a particle's active status may have changed
  model_->get_model_data()->set_check_particles_active(true);
}


//! Add a Float attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
    \param[in] is_optimized Whether the attribute's value should be optimizable.
 */
void Particle::add_attribute(FloatKey name, const Float value,
                             const bool is_optimized)
{
  FloatIndex fi;

  IMP_LOG(VERBOSE, "add_float: " << name);
  IMP_assert(model_ != NULL,
             "Particle must be added to Model before an attributes are added");
  fi = model_->get_model_data()->add_float(value);

  float_indexes_.insert(name, fi);
  model_->get_model_data()->set_is_optimized(fi, is_optimized);
}


//! Does particle have a Float attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Float attribute exists in this particle.
 */
bool Particle::has_attribute(FloatKey name) const
{
  return float_indexes_.contains(name);
}


//! Get the specified Float attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
FloatIndex Particle::get_attribute(FloatKey name) const
{
  return float_indexes_[name];
}


//! Add an Int attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
 */
void Particle::add_attribute(IntKey name, const Int value)
{
  IMP_LOG(VERBOSE, "add_int: " << name);
  IMP_assert(model_ != NULL,
             "Particle must be added to Model before an attributes are added");
  int_indexes_.insert(name, model_->get_model_data()->add_int(value));
}


//! Does particle have an Int attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_attribute(IntKey name) const
{
  return int_indexes_.contains(name);
}


//! Get the specified Int attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
IntIndex Particle::get_attribute(IntKey name) const
{
  return int_indexes_[name];
}


//! Add a String attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
 */
void Particle::add_attribute(StringKey name, const String value)
{
  IMP_assert(model_ != NULL,
             "Particle must be added to Model before an attributes are added");
  string_indexes_.insert(name, model_->get_model_data()->add_string(value));
}


//! Does particle have a String attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_attribute(StringKey name) const
{
  return string_indexes_.contains(name);
}


//! Get the specified String attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
StringIndex Particle::get_attribute(StringKey name) const
{
  return string_indexes_[name];
}


//! Show the particle
/** \param[in] out Stream to write particle description to.
 */
std::ostream& Particle::show(std::ostream& out) const
{
  char* inset = "  ";
  out << std::endl;
  if (is_active_) {
    out << inset << inset << "active";
  } else {
    out << inset << inset << "dead";
  }
  out << std::endl;

  out << inset << inset << "float attributes:" << std::endl;
  float_indexes_.show(out, "    ");

  out << inset << inset << "int attributes:" << std::endl;
  int_indexes_.show(out, "    ");

  out << inset << inset << "string attributes:" << std::endl;
  string_indexes_.show(out, "    ");
  return out;
}

}  // namespace IMP
