/**
 * \file IMP/saxs/RatioVolatilityScore.h \brief Vr score implementation
 *
 * Hura et al. Nature Methods 2013
 *
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
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
class IMPSAXSEXPORT RatioVolatilityScore : public IMP::Object {
 public:
  RatioVolatilityScore(double dmax = 400)
       : IMP::Object("RatioVolatilityScore%1%"), dmax_(dmax) {}

  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       bool use_offset = false) const;

  double compute_scale_factor(const Profile* exp_profile,
                              const Profile* model_profile,
                              double offset = 0.0) const;

  double compute_offset(const Profile* exp_profile,
                        const Profile* model_profile) const {
    // not implemented for now
    IMP_UNUSED(exp_profile);
    IMP_UNUSED(model_profile);
    return 0.0;
  }
  IMP_OBJECT_METHODS(RatioVolatilityScore);

 private:
  double dmax_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RATIO_VOLATILITY_SCORE_H */
