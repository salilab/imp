/**
 *  \file Score.h   \brief A class for SAXS scoring and its derivatives
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_SCORE_H
#define IMPSAXS_SCORE_H

#include "saxs_config.h"
#include "Profile.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

class Profile;
class Restraint;

/**
  SAXS scoring class that allows to compute chi score for the fit between the
  experimental and computational profile. In addition, derivatives with respect
  to the experimental profile can be computed and further used for optimization.
*/
class IMPSAXSEXPORT Score: public RefCounted {
public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  Score(const Profile& exp_profile);

  //! compute chi value
  Float compute_chi_score(const Profile& model_profile,
                          bool use_offset = false,
                          const std::string fit_file_name = "") const {
    return sqrt(compute_chi_square_score(model_profile,
                                         use_offset, fit_file_name));
  }

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  //! fit experimental profile through optimization of c1 and c2 parameters
  /**
     \param[in] partial_profile  partial profiles computed
     \param[in] c1, c2 - the optimal values will be returned in these parameters
     c1 - adjusts the excluded volume, valid range [0.95 - 1.12]
     c2 - adjusts the density of hydration layer, valid range [-4.0 - 4.0]
     \return chi value
  */
  Float fit_profile(Profile partial_profile, float& c1, float& c2,
                    bool fixed_c1 = false, bool fixed_c2 = false,
                    bool use_offset = false,
                    const std::string fit_file_name = "") const;
#endif
  //! fit experimental profile through optimization of c1 and c2 parameters
  Float fit_profile(const Profile& partial_profile,
                    bool use_offset = false,
                    const std::string fit_file_name = "") const {
    // this function version is for python
    float c1, c2;
    return fit_profile(partial_profile, c1, c2, false, false,
                       use_offset, fit_file_name);
  }

  //! compute squared chi value
  Float compute_chi_square_score(const Profile& model_profile,
                                 bool use_offset = false,
                                 const std::string fit_file_name = "") const;


  //! computes scale factor given offset value
  Float compute_scale_factor(const Profile& model_profile,
                             Float offset = 0.0) const;
  // computes offset
  Float compute_offset(const Profile& model_profile) const;

 private:
  // required to fit the q values of computational profile to the experimental
  void resample(const Profile& model_profile, Profile& resampled_profile) const;

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

  IMP_REF_COUNTED_DESTRUCTOR(Score);
  friend class DerivativeCalculator;

 private:
  const Profile exp_profile_;   //  experimental saxs profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_SCORE_H */
