/**
 *  \file SAXSScore.h   \brief A class for SAXS scoring and its derivatives
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_SAXS_SCORE_H
#define IMPSAXS_SAXS_SCORE_H

#include "config.h"
#include "FormFactorTable.h"
#include "RadialDistributionFunction.h"
#include "SAXSProfile.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

/*
 Basic SAXS scoring class
 */

class IMPSAXSEXPORT SAXSScore {
public:
  //! init for theoretical profile
  SAXSScore(FormFactorTable* ff_table,
            SAXSProfile* exp_saxs_profile,
            const std::vector<Particle*>& particles);

  //! compute chi value (assumes the same sampling range!)
  double compute_chi_score(const SAXSProfile& model_saxs_profile);

  //! compute  derivatives on atom iatom - iatom is NOT part of rigid body
  std::vector<IMP::algebra::Vector3D> calculate_chi_derivative(
                           const SAXSProfile& model_saxs_profile,
                           const std::vector<Particle*>& particles) {
    return calculate_chi_real_derivative(model_saxs_profile, particles);
  }

  double get_scaling(void) { return c_; }
  double set_scaling(double c) { c_ = c;  return c_; }

  double get_offset(void) { return offset_; }
  double set_offset(double offset) { offset_ = offset;  return offset_; }

private:
  int init(const std::vector<Particle*>& particles);

  //! compute  derivatives on atom iatom - iatom is NOT part of rigid body
  std::vector<IMP::algebra::Vector3D> calculate_chi_real_derivative (
                           const SAXSProfile& model_saxs_profile,
                           const std::vector<Particle*>& particles);

  void write_SAXS_fit_file(const std::string& file_name,
                           const SAXSProfile& model_saxs_profile) const;

protected:
  FormFactorTable* ff_table_;       // pointer to form factors table
  SAXSProfile* exp_saxs_profile_;   // pointer to experimental saxs profile

  double c_;  // scale constant
  double offset_;   // offset constant
  std::vector< std::vector<double> > sincval_array_;

  //! lookup table for sinc function and cos function
  std::vector<double> sinc_lookup_, cos_lookup_, zero_formfactor_;
  int mesh_sinc_;     //! how many points per 1 unit in sinc
  unsigned int nr_;
  double dr_;

  std::vector<double> r_, r_square_reciprocal_;
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SAXS_SCORE_H */
