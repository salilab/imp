/**
 * \file IMP/SAXSMultiStateModelScore.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_SAXS_MULTI_STATE_MODEL_SCORE_H
#define IMPMULTI_STATE_SAXS_MULTI_STATE_MODEL_SCORE_H

#include "MultiStateModelScore.h"
#include "MultiStateModel.h"

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/WeightedProfileFitter.h>
#include <IMP/base/Object.h>

#include <vector>

IMPMULTISTATE_BEGIN_NAMESPACE

class IMPMULTISTATEEXPORT SAXSMultiStateModelScore :
  public MultiStateModelScore {
public:

  /* if c1_c2_approximate=true, get_score will return approximate score
     based on average c1/c2
     min_weight_threshold, multi-state models with one or more weights below
     min_weight_threshold will get a negative score
  */
  SAXSMultiStateModelScore(const IMP::saxs::Profiles& profiles,
                    const IMP::saxs::Profile* exp_profile,
                    bool c1_c2_approximate,
                    float min_c1 = 0.99, float max_c1 = 1.05,
                    float min_c2 = -0.5, float max_c2 = 2.0);

  double get_score(const MultiStateModel& m) const;

  double get_score(const MultiStateModel& m,
                   std::vector<double>& weights) const;

  IMP::saxs::WeightedFitParameters get_fit_parameters(MultiStateModel& m) const;

  IMP::saxs::WeightedFitParameters get_fit_parameters() const;

  void write_fit_file(MultiStateModel& m,
                      const IMP::saxs::WeightedFitParameters& fp,
                      const std::string fit_file_name) const;

  std::string get_state_name(unsigned int id) const { return profiles_[id]->get_name(); }

  std::string get_dataset_name() const { return exp_profile_->get_name(); }

  float get_average_c1() const { return average_c1_; }
  float get_average_c2() const { return average_c2_; }

  void set_average_c1_c2(const std::vector<IMP::saxs::WeightedFitParameters>& fps);

private:
  void resample(const IMP::saxs::Profile* exp_profile,
                const IMP::saxs::Profiles& profiles,
                IMP::saxs::Profiles& resampled_profiles);

  void set_average_c1_c2(IMP::saxs::WeightedProfileFitter* score,
                         const IMP::saxs::Profiles& profiles);

private:
  // input profiles
  const IMP::saxs::Profiles profiles_;
  IMP::base::PointerMember<const IMP::saxs::Profile> exp_profile_;

  // resampled on experimental profile q's
  IMP::saxs::Profiles resampled_profiles_;

  // scoring with exp_profile_
  IMP::saxs::WeightedProfileFitter* score_;

  float min_c1_, max_c1_, min_c2_, max_c2_;
  float average_c1_, average_c2_;
  bool c1_c2_approximate_;
};

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_SAXS_MULTI_STATE_MODEL_SCORE_H */
