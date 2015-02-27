/**
 *  \file SAXSMultiStateModelScore.cpp
 *
 * \brief A class for scoring SAXS ensembles
 *
 * Author: Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multi_state/SAXSMultiStateModelScore.h>
#include <IMP/multi_state/stat_helpers.h>

#include <IMP/saxs/WeightedProfileFitter.h>

IMPMULTISTATE_BEGIN_NAMESPACE

SAXSMultiStateModelScore::SAXSMultiStateModelScore(
                          const IMP::saxs::Profiles& profiles,
                          const IMP::saxs::Profile* exp_profile,
                          bool c1_c2_approximate,
                          float min_c1, float max_c1,
                          float min_c2, float max_c2) :
  profiles_(profiles), exp_profile_(exp_profile),
  min_c1_(min_c1), max_c1_(max_c1), min_c2_(min_c2), max_c2_(max_c2),
  c1_c2_approximate_(c1_c2_approximate) {

  if(profiles_.size() < 1) {
    IMP_THROW("SAXSMultiStateModelScore - please provide at least one profile"
              << std::endl, IOException);
  }

  // resample all models profiles
  resample(exp_profile_, profiles_, resampled_profiles_);

  // init scoring object
  score_ = new IMP::saxs::WeightedProfileFitter(exp_profile_);

  // compute average c1/c2
  set_average_c1_c2(score_, resampled_profiles_);
}

void SAXSMultiStateModelScore::resample(const IMP::saxs::Profile* exp_profile,
                                 const IMP::saxs::Profiles& profiles,
                                 IMP::saxs::Profiles& resampled_profiles) {

  resampled_profiles.reserve(profiles.size());
  for(unsigned int i=0; i<profiles.size(); i++) {
    IMP::saxs::Profile *resampled_profile =
      new IMP::saxs::Profile(exp_profile->get_min_q(), exp_profile->get_max_q(),
                             exp_profile->get_delta_q());
    profiles[i]->resample(exp_profile, resampled_profile, true);
    resampled_profiles.push_back(resampled_profile);
  }
}

void SAXSMultiStateModelScore::set_average_c1_c2(
                                 IMP::saxs::WeightedProfileFitter* score,
                                 const IMP::saxs::Profiles& profiles) {
  average_c1_ = 0.0;
  average_c2_ = 0.0;
  IMP::saxs::ProfilesTemp profiles_temp(1);
  for(unsigned int i=0; i<profiles.size(); i++) {
    profiles_temp[0] = profiles[i];
    IMP::saxs::WeightedFitParameters fp =
      score->fit_profile(profiles_temp, min_c1_, max_c1_, min_c2_, max_c2_);
    average_c1_ += fp.get_c1();
    average_c2_ += fp.get_c2();
  }
  average_c1_ /= profiles.size();
  average_c2_ /= profiles.size();
}

void SAXSMultiStateModelScore::set_average_c1_c2(
                  const std::vector<IMP::saxs::WeightedFitParameters>& fps) {
  float c1 = 0.0;
  float c2 = 0.0;
  for(unsigned int i=0; i<fps.size(); i++) {
    c1 += fps[i].get_c1();
    c2 += fps[i].get_c2();
  }
  c1 /= fps.size();
  c2 /= fps.size();

  average_c1_ = c1;
  average_c2_ = c2;
}


double SAXSMultiStateModelScore::get_score(const MultiStateModel& m,
                                           std::vector<double>& weights) const {
  const std::vector<unsigned int>& states = m.get_states();
  IMP::saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++) {
    profiles[i] = resampled_profiles_[states[i]];
    if(c1_c2_approximate_)
      profiles[i]->sum_partial_profiles(average_c1_, average_c2_);
  }

  double chi;
  if(c1_c2_approximate_) {
    chi = score_->compute_score(profiles, weights);
  } else { // fit c1/c2
    IMP::saxs::WeightedFitParameters fp =
      score_->fit_profile(profiles, min_c1_, max_c1_, min_c2_, max_c2_);
    chi = fp.get_chi();
  }

  return chi;
}

double SAXSMultiStateModelScore::get_score(const MultiStateModel& m) const {
  std::vector<double> weights;
  return get_score(m, weights);
}


IMP::saxs::WeightedFitParameters
        SAXSMultiStateModelScore::get_fit_parameters(MultiStateModel& m) const {

  const std::vector<unsigned int>& states = m.get_states();
  IMP::saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++)
    profiles[i] = resampled_profiles_[states[i]];

  IMP::saxs::WeightedFitParameters fp =
    score_->fit_profile(profiles, min_c1_, max_c1_, min_c2_, max_c2_);
  m.set_score(fp.get_chi());

  return fp;
}

IMP::saxs::WeightedFitParameters
        SAXSMultiStateModelScore::get_fit_parameters() const {

  IMP::saxs::WeightedFitParameters fp =
    score_->fit_profile(resampled_profiles_, min_c1_, max_c1_, min_c2_, max_c2_);

  return fp;
}

void SAXSMultiStateModelScore::write_fit_file(MultiStateModel& m,
                                     const IMP::saxs::WeightedFitParameters& fp,
                                     const std::string fit_file_name) const {

  const std::vector<unsigned int>& states = m.get_states();
  IMP::saxs::ProfilesTemp profiles(states.size());
  for(unsigned int i=0; i<states.size(); i++)
    profiles[i] = resampled_profiles_[states[i]];
  score_->write_fit_file(profiles, fp, fit_file_name);
}

IMPMULTISTATE_END_NAMESPACE
