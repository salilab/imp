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
#include "Distribution.h"
#include "SAXSProfile.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class SAXSProfile;

/*
 Basic SAXS scoring class
*/

class IMPSAXSEXPORT SAXSScore {
public:
  //! init for theoretical profile
  SAXSScore(FormFactorTable* ff_table, SAXSProfile* exp_saxs_profile);

  //! compute chi value (assumes the same sampling range!)
  Float compute_chi_score(const SAXSProfile& model_saxs_profile,
                          const std::string fit_file_name = "") const {
    return sqrt(compute_chi_square_score(model_saxs_profile, fit_file_name));
  }

  //! compute squared chi value (assumes the same sampling range!)
  Float compute_chi_square_score(const SAXSProfile& model_saxs_profile,
                                 const std::string fit_file_name = "") const;

  //! computes the value of c in order to fit the profiles
  Float compute_fit_coefficient(const SAXSProfile& model_saxs_profile) const;

  //! compute  derivatives on atom iatom - iatom is NOT part of rigid body
  void calculate_chi_derivative(const SAXSProfile& model_saxs_profile,
                  const std::vector<Particle*>& particles,
                  std::vector<IMP::algebra::Vector3D>& derivatioves) const;
 private:
  //  int init(const SAXSProfile& model_saxs_profile);

  // compute derivatives for each particle
  void calculate_chi_real_derivative(const SAXSProfile& model_saxs_profile,
                  const std::vector<Particle*>& particles,
                  std::vector<IMP::algebra::Vector3D>& derivatioves) const;

  void write_SAXS_fit_file(const std::string& file_name,
                           const SAXSProfile& model_saxs_profile,
                           const Float chi_square,
                           const Float c=1, const Float offset=0) const;

  void calculate_profile_difference(const SAXSProfile& model_saxs_profile,
                                    const Float fit_coefficient,
                                    Floats& profile_diff) const;

  void calculate_sinc_cos(Float pr_resolution, Float max_distance,
                          const SAXSProfile& model_saxs_profile,
                          std::vector<Floats>& output_values) const;

  // required to fit the q values of computational profile to the experimental
  void resample(const SAXSProfile& model_saxs_profile,
                SAXSProfile& resampled_profile) const;

  Float compute_fit_coefficient_internal(
                                   const SAXSProfile& model_saxs_profile) const;

  Float compute_chi_square_score_internal(const SAXSProfile& model_saxs_profile,
                                        const std::string& fit_file_name) const;

 protected:
  FormFactorTable* ff_table_;       // pointer to form factors table
  SAXSProfile* exp_saxs_profile_;   // pointer to experimental saxs profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SAXS_SCORE_H */
