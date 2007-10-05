/*
 *  Particle.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__particle_h)
#define __particle_h 1

#include <map>

#include "IMP_config.h"

namespace imp
{

class Model_Data;
class Restraint;

// Particle methods and indexes to particle attributes. Particles can
// be deactivated so that they no longer play a role in model optimization.
// Removing particles and their attributes would cause problems in the way
// attribute values are indexed and should not be done.
class IMPDLLEXPORT Particle
{
public:
  Particle();
  ~Particle();
  void set_model_data(Model_Data* model_data);
  Model_Data* model_data(void);

  // float attributes
  bool add_float(const std::string name, const Float value = 0.0, const bool optimize = false);
  bool has_float(const std::string name);
  Float_Index float_index(const std::string name);

  // int attributes
  bool add_int(const std::string name, const Int value);
  bool has_int(const std::string name);
  Int_Index int_index(const std::string name);

  // string attributes
  bool add_string(const std::string name, const String value);
  bool has_string(const std::string name);
  String_Index string_index(const std::string name);

  // status
  void set_is_active (bool is_active);
  bool is_active (void);

  void show (std::ostream& out = std::cout);

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

#endif
