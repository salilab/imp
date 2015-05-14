/**
 * \file IMP/saxs/ChiScore.h \brief Basic chi score implementation
 *
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_CHI_SCORE_H
#define IMPSAXS_CHI_SCORE_H

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"

IMPSAXS_BEGIN_NAMESPACE

/**
   Basic implementation of Chi scoring
*/
class IMPSAXSEXPORT ChiScore {
 public:
  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       bool use_offset = false) const;

  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                      double min_q, double max_q) const;

  double compute_scale_factor(const Profile* exp_profile,
                              const Profile* model_profile,
                              double offset = 0.0) const;

  double compute_offset(const Profile* exp_profile,
                        const Profile* model_profile) const;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_CHI_SCORE_H */
