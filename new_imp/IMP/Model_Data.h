/*
 *  Model.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__model_data_h)
#define __model_data_h 1

namespace imp
{

class Model;



// All data for particles is stored through indexing of data
// in this structure. Float data is assumed to be potentially
// differentiable and is stored in a structure that can accomodate
// differentiation. Int and string data is stored directly in
// vectors.
class Model_Data
{
  friend class Model;
  friend class Particle;
  friend class Opt_Float_Index_Iterator;

public:
  // variables
  class Float_Data
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

  // particle float attributes (assumed differentiable variables)
  Float_Index add_float(const Float value, const std::string name="none");
  void set_float(const Float_Index idx, const Float value);
  Float get_float(const Float_Index idx);

  void add_to_deriv(const Float_Index idx, const Float value);
  Float get_deriv(const Float_Index idx);
  bool is_optimized(const Float_Index idx);
  void set_is_optimized(const Float_Index idx, bool is_optimized);

  void zero_derivatives(void);

  // particle int attributes
  Int_Index add_int(const Int value);
  void set_int(const Int_Index idx, const Int value);
  Int get_int(const Int_Index idx);

  // particle string attributes
  String_Index add_string(const String value);
  void set_string(const String_Index idx, const String value);
  String get_string(const String_Index idx);

protected:
  Model_Data();
  ~Model_Data();

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
  std::vector<Float_Data> float_data_;
  std::vector<Int> int_data_;
  std::vector<String> string_data_;

  // float attribute state change statistics associated with a particular name in some subset of particles
  std::map<std::string, int> stat_indexes_;
  std::vector<Statistics> float_stats_;

  // flag set whenever a particle is activated or deactivated
  bool check_particles_active_;
};

// variable iterator
class Opt_Float_Index_Iterator
{
public:
  Opt_Float_Index_Iterator() {}
  void reset(Model_Data* model_data);
  bool next(void);
  Float_Index get(void);

protected:
  int cur_;
  Model_Data* model_data_;
};


} // namespace imp

#endif
