/**
 * \file IMP/saxs/ChiScore.h \brief Basic chi score implementation
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
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
class IMPSAXSEXPORT ChiScore : public IMP::Object {
 public:
  ChiScore() : IMP::Object("ChiScore%1%") {}

  // returns Chi_square score
  double compute_score(const Profile* exp_profile, const Profile* model_profile,
                       bool use_offset = false) const;

  double compute_scale_factor(const Profile* exp_profile,
                              const Profile* model_profile,
                              double offset = 0.0) const;

  double compute_offset(const Profile* exp_profile,
                        const Profile* model_profile) const;

  IMP_OBJECT_METHODS(ChiScore);
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_CHI_SCORE_H */
