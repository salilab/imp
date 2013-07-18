/**
 * \file WeightedProfileFitter.h
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
#include "internal/Diagonal.h"
#include "internal/Vector.h"

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT WeightedProfileFitter : public ProfileFitter<ChiScore> {

 public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  WeightedProfileFitter(const Profile& exp_profile);

  //! compute a weighted score that minimizes chi
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
  */
  Float compute_score(const std::vector<IMP::saxs::Profile *>& profiles,
                      std::vector<double>& weights= empty_weights_,
                      const std::string fit_file_name = "") const;

  void compute_weights(const std::vector<IMP::saxs::Profile *>& profiles,
                       std::vector<double>& weights) const;

  //! fit profiles by optimization of c1/c2 and weights
  /**
     it is assumed that the q values of the profiles are the same as
     the q values of the experimental profile. Use Profile::resample to resample
  */
  FitParameters fit_profile(std::vector<IMP::saxs::Profile *>& partial_profiles,
                            float min_c1=0.95, float max_c1=1.05,
                            float min_c2=-2.0, float max_c2=4.0,
                            std::vector<double>& weights = empty_weights_,
                            const std::string fit_file_name = "") const;
private:
  FitParameters search_fit_parameters(
             std::vector<IMP::saxs::Profile *>& partial_profiles,
             float min_c1, float max_c1, float min_c2, float max_c2,
             float old_chi, std::vector<double>& weights) const;

 private:
  internal::Diagonal W_;  // weights matrix

  // weights matrix multiplied by experimental intensities vector
  internal::Vector Wb_;

  //default, when weight are not needed
  static std::vector<double> empty_weights_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_WEIGHTED_PROFILE_FITTER_H */
