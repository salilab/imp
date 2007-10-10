/*
 *  Particle.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PARTICLE_H
#define __IMP_PARTICLE_H

#include <map>

#include "IMP_config.h"
#include "Base_Types.h"
#include "Model_Data.h"
#include "Restraint.h"

namespace imp
{

// Particle methods and indexes to particle attributes. Particles can
// be deactivated so that they no longer play a role in model optimization.
// Removing particles and their attributes would cause problems in the way
// attribute values are indexed and should not be done.
class IMPDLLEXPORT Particle
{
public:
  Particle();
  Particle(Model_Data* model_data);
  ~Particle();
  void set_model_data(Model_Data* model_data);
  Model_Data* model_data(void) const;

  // float attributes
  bool add_float(const std::string name, const Float value = 0.0, const bool optimize = false);
  bool has_float(const std::string name) const;
  Float_Index float_index(const std::string name) const;

  // int attributes
  bool add_int(const std::string name, const Int value);
  bool has_int(const std::string name) const;
  Int_Index int_index(const std::string name) const;

  // string attributes
  bool add_string(const std::string name, const String value);
  bool has_string(const std::string name) const;
  String_Index string_index(const std::string name) const;

  // status
  void set_is_active (bool is_active);
  bool is_active (void) const;

  void show (std::ostream& out = std::cout) const;

protected:
  // all of the particle data
  Model_Data* model_data_;

  bool is_active_; // true if particle is active

  // float attributes associated with the particle
  std::map<std::string, Float_Index> float_indexes_;
  // int attributes associated with the particle
  std::map<std::string, Int_Index> int_indexes_;
  // string attributes associated with the particle
  std::map<std::string, String_Index> string_indexes_;
};

} // namespace imp

#endif  /* __IMP_PARTICLE_H */
