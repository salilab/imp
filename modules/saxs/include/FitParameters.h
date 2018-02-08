/**
 * \file IMP/saxs/FitParameters.h \brief
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_FIT_PARAMETERS_H
#define IMPSAXS_FIT_PARAMETERS_H
#include <IMP/saxs/saxs_config.h>
#include <iostream>

IMPSAXS_BEGIN_NAMESPACE

//! Parameters of a fit, from ProfileFitter.
class IMPSAXSEXPORT FitParameters {
 public:
  FitParameters()
      : chi_(0.0), c1_(0.0), c2_(0.0), c_(0.0), o_(0.0), default_chi_(0.0) {}
  FitParameters(double chi, double c1, double c2, double c, double o)
      : chi_(chi), c1_(c1), c2_(c2), c_(c), o_(o), default_chi_(0.0) {}
  FitParameters(double chi, double c1, double c2)
      : chi_(chi), c1_(c1), c2_(c2), c_(0.0), o_(0.0), default_chi_(0.0) {}

  double get_score() const { return chi_; }
  double get_chi() const { return chi_; }
  double get_c1() const { return c1_; }
  double get_c2() const { return c2_; }
  double get_scale() const { return c_; }
  double get_offset() const { return o_; }
  double get_default_chi() const { return default_chi_; }
  std::string get_pdb_file_name() const { return pdb_file_name_; }
  std::string get_profile_file_name() const { return profile_file_name_; }
  int get_mol_index() const { return mol_index_; }

  void set_chi(double chi) { chi_ = chi; }
  void set_default_chi(double chi) { default_chi_ = chi; }
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
  double chi_;          // fit score
  double c1_;           // excluded volume fit
  double c2_;           // water layer fit
  double c_;            // scaling
  double o_;            // offset
  double default_chi_;  // default chi value without fitting c1/c2
  std::string profile_file_name_;
  std::string pdb_file_name_;
  int mol_index_;  // unique mol index
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_FIT_PARAMETERS_H */
