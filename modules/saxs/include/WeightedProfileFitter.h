/**
 * \file saxs/WeightedProfileFitter.h
 * \brief  Fitting of multiple profiles to the experimental one.
 * The weights of the profiles are computed analytically using
 * non-negative least squares fitting (NNLS)
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_WEIGHTED_PROFILE_FITTER_H
#define IMPSAXS_WEIGHTED_PROFILE_FITTER_H

#include "ProfileFitter.h"
#include "ChiScore.h"
#include "WeightedFitParameters.h"
#include <Eigen/Dense>

IMPSAXS_BEGIN_NAMESPACE

/**
   Fitting of multiple profiles to the experimental one.
   The weights of the profiles are computed analytically using
   non-negative least squares fitting (NNLS).
*/
class IMPSAXSEXPORT WeightedProfileFitter : public ProfileFitter<ChiScore> {

 public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  WeightedProfileFitter(const Profile* exp_profile);

  //! compute a weighted score that minimizes chi
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
     if(NNLS = true, solve non-negative least squares, otherwise solve just
     least squares, that may return negative weights to be discarded later
  */
  Float compute_score(const ProfilesTemp& profiles,
                      std::vector<double>& weights, bool NNLS = true) const;

  //! fit profiles by optimization of c1/c2 and weights
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
  */
  WeightedFitParameters fit_profile(ProfilesTemp partial_profiles,
                                    float min_c1 = 0.95, float max_c1 = 1.05,
                                    float min_c2 = -2.0,
                                    float max_c2 = 4.0) const;

  //! write a fit file
  void write_fit_file(ProfilesTemp partial_profiles,
                      const WeightedFitParameters& fp,
                      const std::string fit_file_name) const;

 private:
  WeightedFitParameters search_fit_parameters(
      ProfilesTemp& partial_profiles, float min_c1, float max_c1, float min_c2,
      float max_c2, float old_chi, std::vector<double>& weights) const;

 private:
  Eigen::MatrixXf W_;  // weights matrix

  // weights matrix multiplied by experimental intensities vector
  Eigen::VectorXf Wb_;

  // intensities
  Eigen::MatrixXf A_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_WEIGHTED_PROFILE_FITTER_H */
