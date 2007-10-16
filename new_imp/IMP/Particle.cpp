/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Particle.h"
#include "mystdexcept.h"
#include "log.h"

namespace IMP
{


//! Constructor
Particle::Particle(): model_data_(NULL)
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
ModelData* Particle::get_model_data(void) const
{
  return model_data_;
}

//! Set pointer to model particle data.
/** This is called by the Model after the particle is added. 
    \param[in] md Pointer to a ModelData object.
 */
void Particle::set_model_data(ModelData *md)
{
  model_data_=md;
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
  model_data_->set_check_particles_active(true);
}


//! Get whether the particle is active.
/** Restraints referencing the particle are only evaluated for 'active'
    particles.
    \return true it the particle is active.
 */
bool Particle::is_active(void) const
{
  return is_active_;
}


//! Add a Float attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
    \param[in] is_optimized Whether the attribute's value should be optimizable.
 */
void Particle::add_float(const std::string name, const Float value,
                         const bool is_optimized)
{
  FloatIndex fi;

  IMP_LOG(VERBOSE, "add_float: " << name);
  IMP_assert(model_data_ != NULL, 
	     "Particle must be added to Model before an attributes are added");
  IMP_assert(!has_float(name), "Trying to add the name '" <<  name << "' twice.");
  // if optimized, give name to get stats generated for this name (e.g. "x", "y" or "z")
  // if (is_optimized)
  //   fi = model_data_->add_float(value, name);
  // else

  // It may be better to manage strings only at the Particle level.
  // You could create the Stat structure here or use an existing
  // one if it already exists for a particular string, then pass
  // a reference to it to the variable.
  fi = model_data_->add_float(value);

  float_indexes_[name] = fi;
  model_data_->set_is_optimized(fi, is_optimized);
}


//! Does particle have a Float attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Float attribute exists in this particle.
 */
bool Particle::has_float(const std::string name) const
{
  return (float_indexes_.find(name) != float_indexes_.end());
}


//! Get the specified Float attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
FloatIndex Particle::get_float_index(const std::string name) const
{
  IMP_check(has_float(name), "Unknown float attribute '" << name << "'", 
	    std::out_of_range("Unknown float attribute name"));
  return float_indexes_.find(name)->second;
}


//! Add an Int attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute.
 */
void Particle::add_int(const std::string name, const Int value)
{
  IMP_LOG(VERBOSE, "add_int: " << name);
  IMP_assert(model_data_ != NULL, 
	     "Particle must be added to Model before an attributes are added");
  IMP_assert(!has_int(name), "Trying to add the name '" <<  name << "' twice.");
  int_indexes_[name] = model_data_->add_int(value);
}


//! Does particle have an Int attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_int(const std::string name) const
{
  return (int_indexes_.find(name) != int_indexes_.end());
}


//! Get the specified Int attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
IntIndex Particle::get_int_index(const std::string name) const
{
  IMP_check(has_int(name), "Unknown int attribute '" << name << "'", 
	    std::out_of_range("Unknown int attribute name"));

  return int_indexes_.find(name)->second;
}


//! Add a String attribute to this particle.
/** \param[in] name Name of the attribute being added.
    \param[in] value Initial value of the attribute. 
 */
void Particle::add_string(const std::string name, const String value)
{
  IMP_assert(model_data_ != NULL, 
		"Particle must be added to Model before an attributes are added");
  IMP_assert(!has_string(name), 
	     "Trying to add the name '" <<  name << "' twice.");
  
  string_indexes_[name] = model_data_->add_string(value);
}


//! Does particle have a String attribute with the given name.
/** \param[in] name Name of the attribute being checked.
    \return true if Int attribute exists in this particle.
 */
bool Particle::has_string(const std::string name) const
{
  return (string_indexes_.find(name) != string_indexes_.end());
}


//! Get the specified String attribute for this particle.
/** \param[in] name Name of the attribute being retrieved.
    \exception std::out_of_range attribute does not exist.
    \return index to the attribute.
 */
StringIndex Particle::get_string_index(const std::string name) const
{
  IMP_check(has_string(name), "Unknown string attribute '" << name << "'", 
	    std::out_of_range("Unknown string attribute name"));
 
  return string_indexes_.find(name)->second;
}


//! Show the particle
/** \param[in] out Stream to write particle description to.
 */
void Particle::show(std::ostream& out) const
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
  std::map<std::string, FloatIndex>::const_iterator iter2;
  for (iter2 = float_indexes_.begin(); iter2 != float_indexes_.end(); ++iter2) {
    out << inset << inset << inset << iter2->first << "  " << model_data_->get_float(iter2->second);
    if (model_data_->get_is_optimized(iter2->second)) {
      out << " (optimized)" << std::endl;
    } else {
      out << std::endl;
    }
  }

  out << inset << inset << "int attributes:" << std::endl;
  std::map<std::string, IntIndex>::const_iterator iter3;
  for (iter3 = int_indexes_.begin(); iter3 != int_indexes_.end(); ++iter3) {
    out << inset << inset << inset << iter3->first << "  " << model_data_->get_int(iter3->second) << std::endl;
  }

  out << inset << inset << "string attributes:" << std::endl;
  std::map<std::string, StringIndex>::const_iterator iter4;
  for (iter4 = string_indexes_.begin(); iter4 != string_indexes_.end(); ++iter4) {
    out << inset << inset << inset << iter4->first << "  " << model_data_->get_string(iter4->second) << std::endl;
  }
}

}  // namespace IMP
