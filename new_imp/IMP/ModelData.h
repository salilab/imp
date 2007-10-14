/*
 *  ModelData.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_DATA_H
#define __IMP_MODEL_DATA_H

#include <vector>
#include <map>

#include "IMP_config.h"
#include "Base_Types.h"

namespace IMP
{

class Model;
class Particle;



// All data for particles is stored through indexing of data
// in this structure. Float data is assumed to be potentially
// differentiable and is stored in a structure that can accomodate
// differentiation. Int and string data is stored directly in
// vectors.
class IMPDLLEXPORT ModelData
{
  friend class Model;
  friend class Particle;
  friend class OptFloatIndexIterator;

  // variables
  class FloatData
  {
  public:
    Float value_;
    Float deriv_;
    int stats_index_;
    bool is_optimized_;
  };

  // variable statistics
  // intended for keeping track of change sizes during optimization
  // ... for efficiency issues (e.g. updates of neighborhoods)
  class Statistics
  {
  public:
    Float min_;
    Float max_;
    Float max_delta_;
    Float min_delta_;
  };
public:


  // particle float attributes (assumed differentiable variables)
  FloatIndex add_float(const Float value);
  void set_float(const FloatIndex idx, const Float value);

  /**
  Get particle float attribute. INLINE

  \param[in] idx Index of the particle float attribute.
  \return value of particle float attribute.
  */
  Float get_float(const FloatIndex idx) const {
    return float_data_[idx.index()].value_;
  }

  void add_to_deriv(const FloatIndex idx, const Float value);
  Float get_deriv(const FloatIndex idx) const;
  bool is_optimized(const FloatIndex idx) const;
  void set_is_optimized(const FloatIndex idx, bool is_optimized);

  void zero_derivatives(void);

  // particle int attributes
  IntIndex add_int(const Int value);
  void set_int(const IntIndex idx, const Int value);

  /**
   Get particle int attribute. INLINE

   \param[in] idx Index of the particle int attribute.
   \return value of particle float attribute.
   */
  Int get_int(const IntIndex idx) const {
    return int_data_[idx.index()];
  }

  // particle string attributes
  StringIndex add_string(const String value);
  void set_string(const StringIndex idx, const String value);

  /**
  Get particle string attribute. INLINE

  \param[in] idx Index of the particle string attribute.
  \return value of particle string attribute.
   */
  String get_string(const StringIndex idx) const {
    return string_data_[idx.index()];
  }

protected:
  ModelData();
  ~ModelData();

  // used by model to see if restraints need to check their particles
  bool check_particles_active(void) {
    return check_particles_active_;
  }
  void set_check_particles_active(bool check_particles_active) {
    check_particles_active_ = check_particles_active;
  }

  // particle variables and attributes
  // these are stored outside of particles to allow
  // ... restraints to get access them directly through
  // ... indexes rather than through particle dereferencing.
  std::vector<FloatData> float_data_;
  std::vector<Int> int_data_;
  std::vector<String> string_data_;

  // float attribute state change statistics associated with a particular name in some subset of particles
  std::map<std::string, int> stat_indexes_;
  std::vector<Statistics> float_stats_;

  // flag set whenever a particle is activated or deactivated
  bool check_particles_active_;
};

// variable iterator
class IMPDLLEXPORT OptFloatIndexIterator
{
public:
  OptFloatIndexIterator() {}
  void reset(ModelData* model_data);
  bool next(void);
  FloatIndex get(void) const;

protected:
  int cur_;
  ModelData* model_data_;
};


} // namespace IMP

#endif  /* __IMP_MODEL_DATA_H */
