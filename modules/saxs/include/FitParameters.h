/**
 * \file IMP/saxs/FitParameters.h \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_FIT_PARAMETERS_H
#define IMPSAXS_FIT_PARAMETERS_H
#include <IMP/saxs/saxs_config.h>
#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT FitParameters {
 public:
  FitParameters()
      : chi_(0.0), c1_(0.0), c2_(0.0), c_(0.0), o_(0.0), default_chi_(0.0) {}
  FitParameters(float chi, float c1, float c2, float c, float o)
      : chi_(chi), c1_(c1), c2_(c2), c_(c), o_(o), default_chi_(0.0) {}
  FitParameters(float chi, float c1, float c2)
      : chi_(chi), c1_(c1), c2_(c2), c_(0.0), o_(0.0), default_chi_(0.0) {}

  float get_score() const { return chi_; }
  float get_chi() const { return chi_; }
  float get_c1() const { return c1_; }
  float get_c2() const { return c2_; }
  float get_scale() const { return c_; }
  float get_offset() const { return o_; }
  float get_default_chi() const { return default_chi_; }
  std::string get_pdb_file_name() const { return pdb_file_name_; }
  std::string get_profile_file_name() const { return profile_file_name_; }
  int get_mol_index() const { return mol_index_; }

  void set_chi(float chi) { chi_ = chi; }
  void set_default_chi(float chi) { default_chi_ = chi; }
  void set_profile_file_name(std::string file_name) {
    profile_file_name_ = file_name;
  }
  void set_pdb_file_name(std::string file_name) { pdb_file_name_ = file_name; }
  void set_mol_index(int index) { mol_index_ = index; }

  void show(std::ostream& s) const {
    s << "Chi = " << chi_ << " c1 = " << c1_ << " c2 = " << c2_
      << " default chi = " << default_chi_ << std::endl;
  }
#ifndef SWIG
  struct compare_fit_parameters {
    bool operator()(const FitParameters& fp1, const FitParameters& fp2) {
      return fp1.get_chi() < fp2.get_chi();
    }
  };
#endif

 protected:
  float chi_;          // fit score
  float c1_;           // excluded volume fit
  float c2_;           // water layer fit
  float c_;            // scaling
  float o_;            // offset
  float default_chi_;  // default chi value without fitting c1/c2
  std::string profile_file_name_;
  std::string pdb_file_name_;
  int mol_index_;  // unique mol index
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_FIT_PARAMETERS_H */
