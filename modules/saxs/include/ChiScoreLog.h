/**
 * \file IMP/saxs/ChiScoreLog.h \brief scoring with log intensity
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_CHI_SCORE_LOG_H
#define IMPSAXS_CHI_SCORE_LOG_H

#include <IMP/warning_macros.h>

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"

IMPSAXS_BEGIN_NAMESPACE

/**
   chi scoring on log intensities
*/
class IMPSAXSEXPORT ChiScoreLog : public IMP::Object {
 public:
  ChiScoreLog() : IMP::Object("ChiScoreLog%1%") {}

  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       bool use_offset = false) const;

  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       double min_q, double max_q) const;

  double compute_scale_factor(const Profile* exp_profile,
                              const Profile* model_profile,
                              double offset = 0.0) const;

  double compute_offset(const Profile* exp_profile,
                        const Profile* model_profile) const {
    // not implemented as no straightforward solution to the equations
    IMP_UNUSED(exp_profile);
    IMP_UNUSED(model_profile);
    return 0.0;
  }

  IMP_OBJECT_METHODS(ChiScoreLog);
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_CHI_SCORE_LOG_H */
