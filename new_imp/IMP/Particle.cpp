/*
 *  Particle.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Particle.h"
#include "mystdexcept.h"
#include "log.h"

namespace imp
{

/**
  Constructor
 */

Particle::Particle ()
{
  LogMsg(VERBOSE, "create particle");
  is_active_ = true;
}


/**
  Destructor
 */

Particle::~Particle ()
{
  LogMsg(VERBOSE, "delete particle");
}


/**
Give accesss to model particle data.

\param[in] model_data All particle data in the model.
*/

void Particle::set_model_data(Model_Data* model_data)
{
  model_data_ = model_data;
}


/**
Get pointer to model particle data.

\return all particle data in the model.
*/

Model_Data* Particle::model_data(void)
{
  return model_data_;
}



/**
  Set whether the particle is active. I.e. if restraints
referencing the particle should be evaluated.

 \param[in] is_active If true, the particle is active.
 */

void Particle::set_is_active(const bool is_active)
{
  is_active_ = is_active;

  // indicate to restraints that a particle's active status may have changed
  model_data_->set_check_particles_active(true);
}


/**
  Get whether the particle is active. I.e. if restraints
referencing the particle should be evaluated.

 \return true it the particle is active.
 */

bool Particle::is_active(void)
{
  return is_active_;
}


/**
  Add a Float attribute to this particle.

  \param[in] name Name of the attribute being added.

  return true if Float attribute successfully added.
 */

bool Particle::add_float (const std::string name, const Float value, const bool is_optimized)
{
  Float_Index fi;

  LogMsg(VERBOSE, "add_float: " << name);
  // if optimized, give name to get stats generated for this name (e.g. "X", "Y" or "Z")
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
  return true;
}


/**
  Does particle have a Float attribute with the given name.

  \param[in] name Name of the attribute being checked.

  return true if Float attribute exists in this particle.
 */


bool Particle::has_float (const std::string name)
{
  return (float_indexes_.find(name) != float_indexes_.end());
}


/**
  Get the specified Float attribute for this particle.

  \param[in] name Name of the attribute being retrieved.

  \return index to the attribute.
 */

Float_Index Particle::float_index(const std::string name)
{
  if (float_indexes_.find(name) == float_indexes_.end()) {
    throw std::out_of_range("Unknown float attribute name");
  }
  return float_indexes_[name];
}


/**
  Add an Int attribute to this particle.

  \param[in] in Input stream to read value from.
  \param[in] name Name of the attribute being added.

  return true if Int attribute successfully added.
 */

bool Particle::add_int (const std::string name, const Int value)
{
  LogMsg(VERBOSE, "add_int: " << name);
  int_indexes_[name] = model_data_->add_int(value);
  return true;
}


/**
  Does particle have an Int attribute with the given name.

  \param[in] name Name of the attribute being checked.

  return true if Int attribute exists in this particle.
 */


bool Particle::has_int (const std::string name)
{
  return (int_indexes_.find(name) != int_indexes_.end());
}


/**
  Get the specified Int attribute for this particle.

  \param[in] name Name of the attribute being retrieved.

  \return index to the attribute.
 */

Int_Index Particle::int_index(const std::string name)
{
  if (int_indexes_.find(name) == int_indexes_.end()) {
    throw std::out_of_range("Unknown int attribute name");
  }
  return int_indexes_[name];
}




/**
  Add a String attribute to this particle.

  \param[in] in Input stream to read value from.
  \param[in] name Name of the attribute being added.

  return true if String attribute successfully added.
 */

bool Particle::add_string(const std::string name, const String value)
{
  string_indexes_[name] = model_data_->add_string(value);
  return true;
}


/**
  Does particle have an String attribute with the given name.

  \param[in] name Name of the attribute being checked.

  return true if String attribute exists in this particle.
 */


bool Particle::has_string (const std::string name)
{
  return (string_indexes_.find(name) != string_indexes_.end());
}



/**
  Get the specified String attribute for this particle.

  \param[in] name Name of the attribute being retrieved.

  \return index to the attribute.
 */

String_Index Particle::string_index(const std::string name)
{
  if (string_indexes_.find(name) == string_indexes_.end()) {
    throw std::out_of_range("Unknown string attribute name");
  }
  return string_indexes_[name];
}


/**
  Show the particle

 \param[in] out Stream to write particle description to.
 */

void Particle::show (std::ostream& out)
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
  std::map<std::string, Float_Index>::iterator iter2;
  for (iter2 = float_indexes_.begin(); iter2 != float_indexes_.end(); ++iter2) {
    out << inset << inset << inset << iter2->first << "  " << model_data_->get_float(iter2->second);
    if (model_data_->is_optimized(iter2->second)) {
      out << " (optimized)" << std::endl;
    } else {
      out << std::endl;
    }
  }

  out << inset << inset << "int attributes:" << std::endl;
  std::map<std::string, Int_Index>::iterator iter3;
  for (iter3 = int_indexes_.begin(); iter3 != int_indexes_.end(); ++iter3) {
    out << inset << inset << inset << iter3->first << "  " << model_data_->get_int(iter3->second) << std::endl;
  }

  out << inset << inset << "string attributes:" << std::endl;
  std::map<std::string, String_Index>::iterator iter4;
  for (iter4 = string_indexes_.begin(); iter4 != string_indexes_.end(); ++iter4) {
    out << inset << inset << inset << iter4->first << "  " << model_data_->get_string(iter4->second) << std::endl;
  }
}

}  // namespace imp
