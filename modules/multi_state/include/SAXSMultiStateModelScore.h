/**
 * \file IMP/multi_state/SAXSMultiStateModelScore.h
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
#include <IMP/Object.h>

#include <vector>

IMPMULTISTATE_BEGIN_NAMESPACE

/** Score multi-state models against SAXS profiles */
class IMPMULTISTATEEXPORT SAXSMultiStateModelScore :
  public MultiStateModelScore {
public:

  /* if c1_c2_approximate=true, get_score will return approximate score
     based on average c1/c2
     min_weight_threshold, multi-state models with one or more weights below
     min_weight_threshold will get a negative score
  */
  SAXSMultiStateModelScore(const saxs::Profiles& profiles,
                           const saxs::Profile* exp_profile,
                           bool c1_c2_approximate,
                           double min_c1 = 0.99, double max_c1 = 1.05,
                           double min_c2 = -0.5, double max_c2 = 2.0);

  double get_score(const MultiStateModel& m) const;

  double get_score(const MultiStateModel& m,
                   Vector<double>& weights) const;

  saxs::WeightedFitParameters get_fit_parameters(MultiStateModel& m) const;

  saxs::WeightedFitParameters get_fit_parameters() const;

  void write_fit_file(MultiStateModel& m,
                      const saxs::WeightedFitParameters& fp,
                      const std::string fit_file_name) const;

  std::string get_state_name(unsigned int id) const { return profiles_[id]->get_name(); }

  std::string get_dataset_name() const { return exp_profile_->get_name(); }

  double get_average_c1() const { return average_c1_; }
  double get_average_c2() const { return average_c2_; }

  void set_average_c1_c2(const Vector<saxs::WeightedFitParameters>& fps);

private:
  void resample(const saxs::Profile* exp_profile,
                const saxs::Profiles& profiles,
                saxs::Profiles& resampled_profiles);

  void set_average_c1_c2(saxs::WeightedProfileFitter* score,
                         const saxs::Profiles& profiles);

private:
  // input profiles
  const saxs::Profiles profiles_;
  IMP::PointerMember<const saxs::Profile> exp_profile_;

  // resampled on experimental profile q's
  saxs::Profiles resampled_profiles_;

  // scoring with exp_profile_
  saxs::WeightedProfileFitter* score_;

  double min_c1_, max_c1_, min_c2_, max_c2_;
  double average_c1_, average_c2_;

  // approximate c1/c2 at get_score(), do accurate fitting at get_fit_parameters()
  bool c1_c2_approximate_;

  // do not perform any c1/c2 fitting
  bool c1_c2_no_fitting_;
};

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_SAXS_MULTI_STATE_MODEL_SCORE_H */
