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
template <typename ScoringFunctionT>
class SAXSMultiStateModelScore : public MultiStateModelScore {
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

  void set_average_c1_c2(saxs::WeightedProfileFitter<ScoringFunctionT>* score,
                         const saxs::Profiles& profiles);

private:
  // input profiles
  const saxs::Profiles profiles_;
  IMP::PointerMember<const saxs::Profile> exp_profile_;

  // resampled on experimental profile q's
  saxs::Profiles resampled_profiles_;

  // scoring with exp_profile_
  saxs::WeightedProfileFitter<ScoringFunctionT>* score_;

  double min_c1_, max_c1_, min_c2_, max_c2_;
  double average_c1_, average_c2_;

  // approximate c1/c2 at get_score(), do accurate fitting at get_fit_parameters()
  bool c1_c2_approximate_;

  // do not perform any c1/c2 fitting
  bool c1_c2_no_fitting_;
};

template <typename ScoringFunctionT>
SAXSMultiStateModelScore<ScoringFunctionT>::SAXSMultiStateModelScore(
                                              const saxs::Profiles& profiles,
                                              const saxs::Profile* exp_profile,
                                              bool c1_c2_approximate,
                                              double min_c1, double max_c1,
                                              double min_c2, double max_c2) :
  profiles_(profiles), exp_profile_(exp_profile),
  min_c1_(min_c1), max_c1_(max_c1), min_c2_(min_c2), max_c2_(max_c2),
  c1_c2_approximate_(c1_c2_approximate), c1_c2_no_fitting_(false) {

  if(profiles_.size() < 1) {
    IMP_THROW("SAXSMultiStateModelScore - please provide at least one profile"
              << std::endl, IOException);
  }

  // resample all models profiles
  resample(exp_profile_, profiles_, resampled_profiles_);

  // init scoring object
  score_ = new saxs::WeightedProfileFitter<ScoringFunctionT>(exp_profile_);

  // compute average c1/c2
  set_average_c1_c2(score_, resampled_profiles_);
}

template <typename ScoringFunctionT>
void SAXSMultiStateModelScore<ScoringFunctionT>::resample(
                                        const saxs::Profile* exp_profile,
                                        const saxs::Profiles& profiles,
                                        saxs::Profiles& resampled_profiles) {

  resampled_profiles.reserve(profiles.size());
  for(unsigned int i=0; i<profiles.size(); i++) {
    saxs::Profile *resampled_profile =
      new saxs::Profile(exp_profile->get_min_q(), exp_profile->get_max_q(),
                             exp_profile->get_delta_q());
    profiles[i]->resample(exp_profile, resampled_profile);
    resampled_profiles.push_back(resampled_profile);
    if(!profiles[i]->is_partial_profile()) c1_c2_no_fitting_ = true;
  }
}

template <typename ScoringFunctionT>
void SAXSMultiStateModelScore<ScoringFunctionT>::set_average_c1_c2(
                             saxs::WeightedProfileFitter<ScoringFunctionT>* score,
                             const saxs::Profiles& profiles) {
  if(c1_c2_no_fitting_) return;
  average_c1_ = 0.0;
  average_c2_ = 0.0;
  saxs::ProfilesTemp profiles_temp(1);
  for(unsigned int i=0; i<profiles.size(); i++) {
    profiles_temp[0] = profiles[i];
    saxs::WeightedFitParameters fp =
      score->fit_profile(profiles_temp, min_c1_, max_c1_, min_c2_, max_c2_);
    average_c1_ += fp.get_c1();
    average_c2_ += fp.get_c2();
  }
  average_c1_ /= profiles.size();
  average_c2_ /= profiles.size();
}

template <typename ScoringFunctionT>
void SAXSMultiStateModelScore<ScoringFunctionT>::set_average_c1_c2(
                               const Vector<saxs::WeightedFitParameters>& fps) {
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
double SAXSMultiStateModelScore<ScoringFunctionT>::get_score(const MultiStateModel& m,
                                           Vector<double>& weights) const {
  const Vector<unsigned int>& states = m.get_states();
  saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++) {
    profiles[i] = resampled_profiles_[states[i]];
    if(c1_c2_approximate_ && !c1_c2_no_fitting_)
      profiles[i]->sum_partial_profiles(average_c1_, average_c2_);
  }

  double chi;
  if(c1_c2_approximate_ || c1_c2_no_fitting_) { // just score calculation
    chi = score_->compute_score(profiles, weights);
  } else { // optimize c1/c2 fit and score
    saxs::WeightedFitParameters fp =
      score_->fit_profile(profiles, min_c1_, max_c1_, min_c2_, max_c2_);
    chi = fp.get_chi();
  }

  return chi;
}

template <typename ScoringFunctionT>
double SAXSMultiStateModelScore<ScoringFunctionT>::get_score(const MultiStateModel& m) const {
  Vector<double> weights;
  return get_score(m, weights);
}


template <typename ScoringFunctionT>
saxs::WeightedFitParameters
        SAXSMultiStateModelScore<ScoringFunctionT>::get_fit_parameters(MultiStateModel& m) const {

  if(c1_c2_no_fitting_) {
    Vector<double> weights;
    double s = get_score(m, weights);
    saxs::WeightedFitParameters wfp(s, 1.0, 0.0, weights);
    return wfp;
  }

  const Vector<unsigned int>& states = m.get_states();
  saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++)
    profiles[i] = resampled_profiles_[states[i]];

  saxs::WeightedFitParameters fp =
    score_->fit_profile(profiles, min_c1_, max_c1_, min_c2_, max_c2_);
  m.set_score(fp.get_chi());

  return fp;
}

template <typename ScoringFunctionT>
saxs::WeightedFitParameters
        SAXSMultiStateModelScore<ScoringFunctionT>::get_fit_parameters() const {

  if(c1_c2_no_fitting_) {
    Vector<double> weights;
    double s = score_->compute_score(resampled_profiles_, weights);
    saxs::WeightedFitParameters wfp(s, 1.0, 0.0, weights);
    return wfp;
  }

  saxs::WeightedFitParameters fp = score_->fit_profile(resampled_profiles_,
                                                       min_c1_, max_c1_,
                                                       min_c2_, max_c2_);
  return fp;
}

template <typename ScoringFunctionT>
void SAXSMultiStateModelScore<ScoringFunctionT>::write_fit_file(MultiStateModel& m,
                                        const saxs::WeightedFitParameters& fp,
                                        const std::string fit_file_name) const {

  const Vector<unsigned int>& states = m.get_states();
  saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++)
    profiles[i] = resampled_profiles_[states[i]];
  score_->write_fit_file(profiles, fp, fit_file_name);
}

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_SAXS_MULTI_STATE_MODEL_SCORE_H */
