/**
 *  \file SAXSScore.h   \brief A class for SAXS scoring and its derivatives
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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

/**
 Basic SAXS scoring class
*/
class IMPSAXSEXPORT SAXSScore {
public:
  //! Constructor
  /**
     \param[in] ff_table Form Factors Table
     \param[in] exp_saxs_profile Experimental profile we want to fit
  */
  SAXSScore(FormFactorTable* ff_table, SAXSProfile* exp_saxs_profile);

  //! compute chi value
  Float compute_chi_score(const SAXSProfile& model_saxs_profile,
                          const std::string fit_file_name = "",
                          bool use_offset = false) const {
    return sqrt(
     compute_chi_square_score(model_saxs_profile, fit_file_name, use_offset));
  }

  //! compute squared chi value
  Float compute_chi_square_score(const SAXSProfile& model_saxs_profile,
                                 const std::string fit_file_name = "",
                                 bool use_offset = false) const;

  //! compute derivatives for each Particle
  /**
     \param[in] model_saxs_profile The current profile of particles
     \param[in] particles Derivative will be computed for each particle
     \param[in] derivatives Output vector
     \param[in] use_offset
     It returns the chi square score for current particle setup
  */
  Float compute_chi_derivative(const SAXSProfile& model_saxs_profile,
                               const std::vector<Particle*>& particles,
                               std::vector<algebra::Vector3D>& derivatives,
                               bool use_offset = false) const;
 private:

  // required to fit the q values of computational profile to the experimental
  void resample(const SAXSProfile& model_saxs_profile,
                SAXSProfile& resampled_profile) const;

  // computes scale factor given offset value
  Float compute_scale_factor(const SAXSProfile& model_saxs_profile,
                             Float offset = 0.0) const;

  // computes offset
  Float compute_offset(const SAXSProfile& model_saxs_profile) const;

  // computes chi square
  Float compute_chi_square_score_internal(const SAXSProfile& model_saxs_profile,
                                        const std::string& fit_file_name,
                                        bool use_offset = false) const;

  // computes chi square given scale factor c and offset
  Float compute_chi_square_score_internal(const SAXSProfile& model_saxs_profile,
                                       const Float c, const Float offset) const;

  // writes 3 column fit file, given scale factor c, offset and chi square
  void write_SAXS_fit_file(const std::string& file_name,
                           const SAXSProfile& model_saxs_profile,
                           const Float chi_square,
                           const Float c=1, const Float offset=0) const;

  // compute derivatives for each particle
  Float compute_chi_real_derivative(const SAXSProfile& model_saxs_profile,
                              const std::vector<Particle*>& particles,
                              std::vector<algebra::Vector3D>& derivatives,
                              bool use_offset) const;

  // helper function for derivatives
  void compute_profile_difference(const SAXSProfile& model_saxs_profile,
                                  const Float c, const Float offset,
                                  Floats& profile_diff) const;

  // helper function for derivatives
  void compute_sinc_cos(Float pr_resolution, Float max_distance,
                        const SAXSProfile& model_saxs_profile,
                        std::vector<Floats>& output_values) const;

 protected:
  FormFactorTable* ff_table_;       // pointer to form factors table
  SAXSProfile* exp_saxs_profile_;   // pointer to experimental saxs profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SAXS_SCORE_H */
