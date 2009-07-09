/**
 *  \file Score.h   \brief A class for SAXS scoring and its derivatives
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_SCORE_H
#define IMPSAXS_SCORE_H

#include "config.h"
#include "FormFactorTable.h"
#include "Distribution.h"
#include "Profile.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class Profile;

/**
  SAXS scoring class that allows to compute chi score for the fit between the
  experimental and computational profile. In addition, derivatives with respect
  to the experimental profile can be computed and further used for optimization.
*/
class IMPSAXSEXPORT Score {
public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
     \param[in] ff_table Form Factors Table
  */
  Score(Profile* exp_profile,
        FormFactorTable* ff_table = default_form_factor_table());

  //! compute chi value
  Float compute_chi_score(const Profile& model_profile,
                          bool use_offset = false,
                          const std::string fit_file_name = "") const {
    return sqrt(compute_chi_square_score(model_profile,
                                         use_offset, fit_file_name));
  }

  //! compute squared chi value
  Float compute_chi_square_score(const Profile& model_profile,
                                 bool use_offset = false,
                                 const std::string fit_file_name = "") const;

  //! compute derivatives for each Particle in particles
  /**
     \param[in] model_profile The current profile of particles
     \param[in] particles Derivative will be computed for each particle
     \param[in] derivatives Output vector
     \param[in] use_offset if true, non zero offset for fitting is used
     \return chi square score for current particle setup
  */
  void compute_chi_derivative(const Profile& model_profile,
                              const Particles& particles,
                              std::vector<algebra::Vector3D>& derivatives,
                              bool use_offset = false) const {
    return compute_chi_derivative(model_profile, particles, particles,
                                  derivatives, use_offset);
  }

  //! compute derivatives for particles1 with respect to particles2
  /**
     This method is needed for rigid bodies when particles1 and particles2
     are particles of two rigid bodies. In this case the particles of the
     same rigid body are ignored.
     \param[in] model_profile The current profile of particles
     \param[in] particles1 Derivative will be computed for each particle
     \param[in] particles2 Derivative will be computed relative to this set
     \param[in] derivatives Output vector
     \param[in] use_offset if true, non zero offset for fitting is used
     \return chi square score for current particle setup
  */
  void compute_chi_derivative(const Profile& model_profile,
                              const Particles& particles1,
                              const Particles& particles2,
                              std::vector<algebra::Vector3D>& derivatives,
                              bool use_offset = false) const;
 private:

  // required to fit the q values of computational profile to the experimental
  void resample(const Profile& model_profile, Profile& resampled_profile) const;

  // computes scale factor given offset value
  Float compute_scale_factor(const Profile& model_profile,
                             Float offset = 0.0) const;

  // computes offset
  Float compute_offset(const Profile& model_profile) const;

  // computes chi square
  Float compute_chi_square_score_internal(const Profile& model_profile,
                                          const std::string& fit_file_name,
                                          bool use_offset = false) const;

  // computes chi square given scale factor c and offset
  Float compute_chi_square_score_internal(const Profile& model_profile,
                                       const Float c, const Float offset) const;

  // writes 3 column fit file, given scale factor c, offset and chi square
  void write_SAXS_fit_file(const std::string& file_name,
                           const Profile& model_profile,
                           const Float chi_square,
                           const Float c=1, const Float offset=0) const;

  // compute derivatives for each particle
  void compute_chi_real_derivative(const Profile& model_profile,
                                   const Particles& particles1,
                                   const Particles& particles2,
                                   std::vector<algebra::Vector3D>& derivatives,
                                   bool use_offset) const;

  // helper function for derivatives
  void compute_profile_difference(const Profile& model_profile,
                                  const Float c, const Float offset,
                                  Floats& profile_diff) const;

  // helper function for derivatives
  void compute_sinc_cos(Float pr_resolution, Float max_distance,
                        const Profile& model_profile,
                        std::vector<Floats>& output_values) const;

 protected:
  FormFactorTable* ff_table_;       // pointer to form factors table
  Profile* exp_profile_;   // pointer to experimental saxs profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SCORE_H */
