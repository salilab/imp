/**
 * \file IMP/multi_state/SAXSMultiCombinationScore.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_SAXS_MULTI_COMBINATION_SCORE_H
#define IMPMULTI_STATE_SAXS_MULTI_COMBINATION_SCORE_H

#include "MultiStateModelScore.h"
#include "MultiStateModel.h"

#include <IMP/saxs/Profile.h>
#include <IMP/saxs/ProfileFitter.h>
#include <IMP/Object.h>

#include <vector>

IMPMULTISTATE_BEGIN_NAMESPACE

/** Score multi-state models against SAXS profiles */
template <typename ScoringFunctionT>
class SAXSMultiCombinationScore : public MultiStateModelScore {
public:

  /* if c1_c2_approximate=true, get_score will return approximate score
     based on average c1/c2
     min_weight_threshold, multi-state models with one or more weights below
     min_weight_threshold will get a negative score
  */
  SAXSMultiCombinationScore(const saxs::Profile* main_profile,
                            const saxs::Profiles& profiles,
                           const saxs::Profile* exp_profile,
                           bool c1_c2_approximate,
                           double min_c1 = 0.99, double max_c1 = 1.05,
                           double min_c2 = -0.5, double max_c2 = 2.0,
                           bool use_offset = false);

  double get_score(const MultiStateModel& m) const override;
  double get_score(const MultiStateModel& m,
                  Vector<double>& weights) const override {
    IMP_UNUSED(weights);
    return get_score(m);
  }

  saxs::WeightedFitParameters get_fit_parameters(MultiStateModel& m) const override;

  saxs::WeightedFitParameters get_fit_parameters() const override;

  void write_fit_file(MultiStateModel& m,
                      const saxs::WeightedFitParameters& fp,
                      const std::string fit_file_name) const override;

  std::string get_state_name(unsigned int id) const override {
    return profiles_[id]->get_name();
  }

  std::string get_dataset_name() const override {
    return exp_profile_->get_name();
  }

  double get_average_c1() const { return average_c1_; }
  double get_average_c2() const { return average_c2_; }

  void set_average_c1_c2(const Vector<saxs::FitParameters>& fps);

private:

private:
  // input profiles
  const saxs::Profiles profiles_; // all the others relative to main
  const saxs::Profile* main_profile_;
  IMP::PointerMember<const saxs::Profile> exp_profile_;

  // scoring with exp_profile_
  saxs::ProfileFitter<ScoringFunctionT>* score_;

  double min_c1_, max_c1_, min_c2_, max_c2_;
  double average_c1_, average_c2_;

  // approximate c1/c2 at get_score(), do accurate fitting at get_fit_parameters()
  bool c1_c2_approximate_;

  // do not perform any c1/c2 fitting
  bool c1_c2_no_fitting_;

  bool use_offset_;
};

template <typename ScoringFunctionT>
SAXSMultiCombinationScore<ScoringFunctionT>::SAXSMultiCombinationScore(
                                              const saxs::Profile* main_profile,
                                              const saxs::Profiles& profiles,
                                              const saxs::Profile* exp_profile,
                                              bool c1_c2_approximate,
                                              double min_c1, double max_c1,
                                              double min_c2, double max_c2,
                                              bool use_offset) :
  profiles_(profiles), main_profile_(main_profile), exp_profile_(exp_profile),
  min_c1_(min_c1), max_c1_(max_c1), min_c2_(min_c2), max_c2_(max_c2),
  c1_c2_approximate_(c1_c2_approximate), c1_c2_no_fitting_(false),
  use_offset_(use_offset) {

  if(profiles_.size() < 1) {
    IMP_THROW("SAXSMultiCombinationScore - please provide at least one profile"
              << std::endl, IOException);
  }

  // init scoring object
  score_ = new saxs::ProfileFitter<ScoringFunctionT>(exp_profile_);

  // compute average c1/c2
  //set_average_c1_c2(score_, resampled_profiles_);
  average_c1_ = 1.02;
  average_c2_ = 1.0;
}

template <typename ScoringFunctionT>
void SAXSMultiCombinationScore<ScoringFunctionT>::set_average_c1_c2(
                               const Vector<saxs::FitParameters>& fps) {
  if(c1_c2_no_fitting_) return;
  double c1 = 0.0;
  double c2 = 0.0;
  for(unsigned int i=0; i<fps.size(); i++) {
    c1 += fps[i].get_c1();
    c2 += fps[i].get_c2();
  }
  c1 /= fps.size();
  c2 /= fps.size();

  average_c1_ = c1;
  average_c2_ = c2;
}


template <typename ScoringFunctionT>
double SAXSMultiCombinationScore<ScoringFunctionT>::get_score(const MultiStateModel& m) const {
  const Vector<unsigned int>& states = m.get_states();

  IMP_NEW(saxs::Profile, combined_profile,(main_profile_->get_min_q(),
                                           main_profile_->get_max_q(),
                                           main_profile_->get_delta_q()));

  // get the main part profile
  combined_profile->add_partial_profiles(main_profile_);

  // add the partial profiles
  for(unsigned int i=0; i<states.size(); i++) {
    combined_profile->add_partial_profiles(profiles_[states[i]]);
  }

  saxs::FitParameters fp =
    score_->fit_profile(combined_profile, min_c1_, max_c1_, min_c2_, max_c2_, use_offset_);
  return fp.get_chi_square();
}

template <typename ScoringFunctionT>
saxs::WeightedFitParameters
        SAXSMultiCombinationScore<ScoringFunctionT>::get_fit_parameters(MultiStateModel& m) const {

  if(c1_c2_no_fitting_) {
    double s = get_score(m);
    saxs::WeightedFitParameters fp(s, 1.0, 0.0);
    return fp;
  }
  const Vector<unsigned int>& states = m.get_states();
  IMP_NEW(saxs::Profile, combined_profile,(main_profile_->get_min_q(),
                                           main_profile_->get_max_q(),
                                           main_profile_->get_delta_q()));

   // get the main part profile
  combined_profile->add_partial_profiles(main_profile_);

  // add the partial profiles
  for(unsigned int i=0; i<states.size(); i++) {
    combined_profile->add_partial_profiles(profiles_[states[i]]);
  }

  saxs::WeightedFitParameters fp =
    score_->fit_profile(combined_profile, min_c1_, max_c1_, min_c2_, max_c2_, use_offset_);
  Vector<double> weights(states.size(), 1.0/(states.size()+1));
  fp.set_weights(weights);
  m.set_score(fp.get_chi_square());
  return fp;
}

template <typename ScoringFunctionT>
saxs::WeightedFitParameters
        SAXSMultiCombinationScore<ScoringFunctionT>::get_fit_parameters() const {

  IMP_NEW(saxs::Profile, combined_profile,(main_profile_->get_min_q(),
                                           main_profile_->get_max_q(),
                                           main_profile_->get_delta_q()));

  // get the main part profile
  combined_profile->add_partial_profiles(main_profile_);

  // add the partial profiles
  for(unsigned int i=0; i<profiles_.size(); i++) {
    combined_profile->add_partial_profiles(profiles_[i]);
  }

  if(c1_c2_no_fitting_) {
    double s = score_->compute_score(combined_profile, use_offset_);
    saxs::WeightedFitParameters fp(s, 1.0, 0.0);
    return fp;
  }

  saxs::WeightedFitParameters fp = score_->fit_profile(combined_profile,
                                               min_c1_, max_c1_,
                                               min_c2_, max_c2_, use_offset_);
  return fp;
}

template <typename ScoringFunctionT>
void SAXSMultiCombinationScore<ScoringFunctionT>::write_fit_file(MultiStateModel& m,
                                        const saxs::WeightedFitParameters& fp,
                                        const std::string fit_file_name) const {

  IMP_UNUSED(fp);
  const Vector<unsigned int>& states = m.get_states();

  IMP_NEW(saxs::Profile, combined_profile,(main_profile_->get_min_q(),
                                           main_profile_->get_max_q(),
                                           main_profile_->get_delta_q()));

   // get the main part profile
  combined_profile->add_partial_profiles(main_profile_);

  // add the partial profiles
  for(unsigned int i=0; i<states.size(); i++) {
    combined_profile->add_partial_profiles(profiles_[states[i]]);
  }

  saxs::WeightedFitParameters fpp = score_->fit_profile(combined_profile,
                                                        min_c1_, max_c1_,
                                                        min_c2_, max_c2_, use_offset_, fit_file_name);
}

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_SAXS_MULTI_COMBINATION_SCORE_H */
