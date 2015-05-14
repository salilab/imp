/**
 * \file IMP/saxs/RatioVolatilityScore.h \brief Vr score implementation
 *
 * Hura et al. Nature Methods 2013
 *
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RATIO_VOLATILITY_SCORE_H
#define IMPSAXS_RATIO_VOLATILITY_SCORE_H

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"

IMPSAXS_BEGIN_NAMESPACE

/**
   Basic implementation of RatioVolatility scoring
*/
class IMPSAXSEXPORT RatioVolatilityScore {
 public:
  RatioVolatilityScore(double dmax = 400) : dmax_(dmax) {}

  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       bool use_offset = false) const;

  double compute_scale_factor(const Profile* exp_profile,
                              const Profile* model_profile,
                              double offset = 0.0) const;

  double compute_offset(const Profile* exp_profile,
                        const Profile* model_profile) const;

 private:
  double dmax_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RATIO_VOLATILITY_SCORE_H */
