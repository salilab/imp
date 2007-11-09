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
  IMP_assert(!has_attribute(name), "Trying to add the name '" 
             <<  name << "' twice. Particle state is " << *this);

  // It may be better to manage strings only at the Particle level.
  // You could create the Stat structure here or use an existing
  // one if it already exists for a particular string, then pass
  // a reference to it to the variable.
  fi = model_->get_model_data()->add_float(value);

  float_indexes_[name] = fi;
  model_->get_model_data()->set_is_optimized(fi, is_optimized);
}


//! Does particle have a Float attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Float attribute exists in this particle.
 */
bool Particle::has_attribute(FloatKey name) const
{
  return (float_indexes_.find(name) != float_indexes_.end());
}


//! Get the specified Float attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
FloatIndex Particle::get_attribute(FloatKey name) const
{
  IMP_check(has_attribute(name), "Unknown float attribute '" << name 
            << "'. Particle state is " << *this, 
            std::out_of_range(std::string("Unknown float attribute name ")
                              + name.get_string()));
  return float_indexes_.find(name)->second;
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
  IMP_assert(!has_attribute(name), "Trying to add the name '"
             <<  name << "' twice. Particle state is " << *this);
  int_indexes_[name] = model_->get_model_data()->add_int(value);
}


//! Does particle have an Int attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_attribute(IntKey name) const
{
  return (int_indexes_.find(name) != int_indexes_.end());
}


//! Get the specified Int attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
IntIndex Particle::get_attribute(IntKey name) const
{
  IMP_check(has_attribute(name), "Unknown int attribute '" << name 
            << "'. Particle state is " << *this, 
            std::out_of_range(std::string("Unknown int attribute name")
                              + name.get_string()));

  return int_indexes_.find(name)->second;
}


//! Add a String attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute. 
 */
void Particle::add_attribute(StringKey name, const String value)
{
  IMP_assert(model_ != NULL, 
             "Particle must be added to Model before an attributes are added");
  IMP_assert(!has_attribute(name), 
             "Trying to add the name '" 
             <<  name << "' twice. Particle state is " << *this);

  string_indexes_[name] = model_->get_model_data()->add_string(value);
}


//! Does particle have a String attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_attribute(StringKey name) const
{
  return (string_indexes_.find(name) != string_indexes_.end());
}


//! Get the specified String attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
StringIndex Particle::get_attribute(StringKey name) const
{
  IMP_check(has_attribute(name), "Unknown string attribute '" << name 
            << "'. Particle state is " << *this, 
            std::out_of_range(std::string("Unknown string attribute name")
                              + name.get_string()));

  return string_indexes_.find(name)->second;
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
  std::map<FloatKey, FloatIndex>::const_iterator iter2;
  for (iter2 = float_indexes_.begin(); iter2 != float_indexes_.end(); ++iter2) {
    out << inset << inset << inset << iter2->first << "  "
        << model_->get_model_data()->get_value(iter2->second);
    if (model_->get_model_data()->get_is_optimized(iter2->second)) {
      out << " (optimized)" << std::endl;
    } else {
      out << std::endl;
    }
  }

  out << inset << inset << "int attributes:" << std::endl;
  std::map<IntKey, IntIndex>::const_iterator iter3;
  for (iter3 = int_indexes_.begin(); iter3 != int_indexes_.end(); ++iter3) {
    out << inset << inset << inset << iter3->first << "  "
        << model_->get_model_data()->get_value(iter3->second) << std::endl;
  }

  out << inset << inset << "string attributes:" << std::endl;
  std::map<StringKey, StringIndex>::const_iterator iter4;
  for (iter4 = string_indexes_.begin(); iter4 != string_indexes_.end();
       ++iter4) {
    out << inset << inset << inset << iter4->first << "  "
        << model_->get_model_data()->get_value(iter4->second) << std::endl;
  }
  return out;
}

}  // namespace IMP
