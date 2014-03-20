/**
 * \file IMP/saxs/ChiFreeScore.h \brief Chi free score implementation
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 * TODO: add reference
 */

#ifndef IMPSAXS_CHI_FREE_SCORE_H
#define IMPSAXS_CHI_FREE_SCORE_H

#include <IMP/saxs/saxs_config.h>
#include "Profile.h"

IMPSAXS_BEGIN_NAMESPACE
/**
   Implementation of chi free score
   Accurate assessment of mass, models and resolution by small-angle scattering.
   Rambo RP, Tainer JA. Nature. 2013
*/
class IMPSAXSEXPORT ChiFreeScore {
 public:
  ChiFreeScore(unsigned int ns, unsigned int k) : ns_(ns), K_(k) {
    if (K_ % 2 == 0) K_++;  // make sure it is odd for median
    last_scale_updated_ = false;
  }

  Float compute_score(const Profile* exp_profile, const Profile* model_profile,
                      bool use_offset = false) const;

  Float compute_scale_factor(const Profile* exp_profile,
                             const Profile* model_profile,
                             Float offset = 0.0) const;

  Float compute_offset(const Profile* exp_profile,
                       const Profile* model_profile) const;

 private:
  unsigned int ns_;  // number of Shannon channels
  unsigned int K_;
  Float last_scale_;
  bool last_scale_updated_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_CHI_FREE_SCORE_H */
