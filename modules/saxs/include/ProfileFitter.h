/**
 * \file IMP/saxs/ProfileFitter.h \brief a class for fitting two profiles
 *
 * Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_FITTER_H
#define IMPSAXS_PROFILE_FITTER_H

#include "ChiScore.h"
#include "FitParameters.h"
#include "Profile.h"
#include <IMP/Object.h>

#include <fstream>

IMPSAXS_BEGIN_NAMESPACE

//! Fit two profiles with user-defined scoring function as a template parameter.
/** By default chi score is used.
    The scoring function template parameter class has to implement three
    basic functions: compute_score, compute_scale_factor and compute_offset.
    see ChiScore for example.
    Currently four scoring functions are implemented:
    ChiScore, ChiScoreLog, ChiFreeScore, and RatioVolatilityScore
 */
template <typename ScoringFunctionT = ChiScore>
class ProfileFitter : public Object {
 public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  ProfileFitter(const Profile* exp_profile)
      : Object("ProfileFitter%1%"), exp_profile_(exp_profile) {
    set_was_used(true);
    scoring_function_ = new ScoringFunctionT();
  }

  ProfileFitter(const Profile* exp_profile, ScoringFunctionT* sf)
      : Object("ProfileFitter%1%"), exp_profile_(exp_profile) {
    set_was_used(true);
    scoring_function_ = sf;
  }

  //! compute fit score
  double compute_score(const Profile* model_profile, bool use_offset = false,
                       const std::string fit_file_name = "") const;

  //! fit experimental profile through optimization of c1 and c2 parameters
  /**
     c1 - adjusts the excluded volume, valid range [0.95 - 1.05]
     c2 - adjusts the density of hydration layer, valid range [-2.0 - 4.0]
     \param[in] partial_profile  partial profiles computed
     \param[in] min_c1 minimal c1 value
     \param[in] max_c1 maximal c1 value
     \param[in] min_c2 minimal c2 value
     \param[in] max_c2 maximal c2 value
     \param[in] use_offset use offset in fitting
     \param[in] fit_file_name write fit in the given filename,
                nothing is written if empty
     \return FitParameters (score, c1, c2)
  */
  FitParameters fit_profile(Profile* partial_profile, double min_c1 = 0.95,
                            double max_c1 = 1.05, double min_c2 = -2.0,
                            double max_c2 = 4.0, bool use_offset = false,
                            const std::string fit_file_name = "") const;

  //! computes the scaling factor needed for fitting the modeled profile
  // onto the experimental one
  /** resampling of the modeled profile is required prior to calling
      to this function, in order to match the q values of the exp. profile.
      this is not done by default to minimize the number of calls to resample.
   */
  double compute_scale_factor(const Profile* model_profile,
                              double offset = 0.0) const {
    return scoring_function_->compute_scale_factor(exp_profile_, model_profile,
                                                   offset);
  }

  //! computes offset for fitting for fitting the modeled profile
  // onto the experimental one
  /** resampling of the modeled profile is required prior to calling
      to this function, in order to match the q values of the exp. profile.
      this is not done by default to minimize the number of calls to resample.
  */
  double compute_offset(const Profile* model_profile) const {
    return scoring_function_->compute_offset(exp_profile_, model_profile);
  }

  //! resampling of the modeled profile is required to fit the q
  // values of the computational profile to the experimental one
  void resample(const Profile* model_profile, Profile* resampled_profile) const;

  // writes 3 column fit file, given scale factor c, offset and chi square
  void write_SAXS_fit_file(const std::string& file_name,
                           const Profile* model_profile, const double chi_square,
                           const double c = 1, const double offset = 0) const;

 private:
  FitParameters search_fit_parameters(Profile* partial_profile, double min_c1,
                                      double max_c1, double min_c2, double max_c2,
                                      bool use_offset, double old_chi) const;

  IMP_REF_COUNTED_DESTRUCTOR(ProfileFitter);
  friend class DerivativeCalculator;

 protected:
  //  experimental saxs profile
  PointerMember<const Profile> exp_profile_;
  PointerMember<ScoringFunctionT> scoring_function_;
};

template <typename ScoringFunctionT>
void ProfileFitter<ScoringFunctionT>::resample(
    const Profile* model_profile, Profile* resampled_profile) const {
  model_profile->resample(exp_profile_, resampled_profile);
}

template <typename ScoringFunctionT>
FitParameters ProfileFitter<ScoringFunctionT>::search_fit_parameters(
    Profile* partial_profile, double min_c1, double max_c1, double min_c2,
    double max_c2, bool use_offset, double old_chi) const {
  int c1_cells = 10;
  int c2_cells = 10;
  if (old_chi < (std::numeric_limits<double>::max() - 1)) {  // second iteration
    c1_cells = 5;
    c2_cells = 5;
  }

  double delta_c1 = (max_c1 - min_c1) / c1_cells;
  double delta_c2 = (max_c2 - min_c2) / c2_cells;

  bool last_c1 = false;
  bool last_c2 = false;
  if (delta_c1 < 0.0001) {
    c1_cells = 1;
    delta_c1 = max_c1 - min_c1;
    last_c1 = true;
  }
  if (delta_c2 < 0.001) {
    c2_cells = 1;
    delta_c2 = max_c2 - min_c2;
    last_c2 = true;
  }
  double best_c1(1.0), best_c2(0.0), best_chi(old_chi);
  bool best_set = false;

  double c1(min_c1);
  for (int i = 0; i <= c1_cells; i++, c1 += delta_c1) {
    double c2 = min_c2;
    for (int j = 0; j <= c2_cells; j++, c2 += delta_c2) {
      partial_profile->sum_partial_profiles(c1, c2);
      double curr_chi = compute_score(partial_profile, use_offset);
      if (!best_set || curr_chi < best_chi) {
        best_set = true;
        best_chi = curr_chi;
        best_c1 = c1;
        best_c2 = c2;
      }
    }
  }

  if (std::fabs(best_chi - old_chi) > 0.0001 &&
      (!(last_c1 && last_c2))) {  // refine more
    min_c1 = std::max(best_c1 - delta_c1, min_c1);
    max_c1 = std::min(best_c1 + delta_c1, max_c1);
    min_c2 = std::max(best_c2 - delta_c2, min_c2);
    max_c2 = std::min(best_c2 + delta_c2, max_c2);
    return search_fit_parameters(partial_profile, min_c1, max_c1, min_c2,
                                 max_c2, use_offset, best_chi);
  }
  return FitParameters(best_chi, best_c1, best_c2);
}

template <typename ScoringFunctionT>
FitParameters ProfileFitter<ScoringFunctionT>::fit_profile(
    Profile* partial_profile, double min_c1, double max_c1, double min_c2,
    double max_c2, bool use_offset, const std::string fit_file_name) const {

  // compute chi value for default c1/c1 (remove?)
  double default_c1 = 1.0, default_c2 = 0.0;
  partial_profile->sum_partial_profiles(default_c1, default_c2);
  double default_chi = compute_score(partial_profile, use_offset);

  FitParameters fp =
      search_fit_parameters(partial_profile, min_c1, max_c1, min_c2, max_c2,
                            use_offset, std::numeric_limits<double>::max());
  double best_c1 = fp.get_c1();
  double best_c2 = fp.get_c2();
  fp.set_default_chi_square(default_chi);

  // compute a profile for best c1/c2 combination
  partial_profile->sum_partial_profiles(best_c1, best_c2);
  compute_score(partial_profile, use_offset, fit_file_name);

  return fp;
}

template <typename ScoringFunctionT>
double ProfileFitter<ScoringFunctionT>::compute_score(
    const Profile* model_profile, bool use_offset,
    const std::string fit_file_name) const {
  IMP_NEW(Profile, resampled_profile,
          (exp_profile_->get_min_q(), exp_profile_->get_max_q(),
           exp_profile_->get_delta_q()));
  model_profile->resample(exp_profile_, resampled_profile);

  double score = scoring_function_->compute_score(exp_profile_,
                                                  resampled_profile,
                                                  use_offset);
  if (fit_file_name.length() > 0) {
    double offset = 0.0;
    if (use_offset)
      offset =
        scoring_function_->compute_offset(exp_profile_, resampled_profile);
    double c = scoring_function_->compute_scale_factor(exp_profile_,
                                                       resampled_profile,
                                                       offset);
    write_SAXS_fit_file(fit_file_name, resampled_profile, score, c, offset);
  }
  return score;
}

template <typename ScoringFunctionT>
void ProfileFitter<ScoringFunctionT>::write_SAXS_fit_file(
    const std::string& file_name, const Profile* model_profile,
    const double score, const double c, const double offset) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name, IOException);
  }

  unsigned int profile_size =
      std::min(model_profile->size(), exp_profile_->size());
  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << profile_size
           << ", q_min = " << exp_profile_->get_min_q()
           << ", q_max = " << exp_profile_->get_max_q();
  out_file << ", delta_q = " << exp_profile_->get_delta_q() << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file << "# offset = " << offset << ", scaling c = " << c
           << ", Chi^2 = " << score << std::endl;
  out_file << "#  q       exp_intensity   model_intensity error" << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_size; i++) {
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(8);
    out_file << exp_profile_->get_q(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << exp_profile_->get_intensity(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << model_profile->get_intensity(i) * c - offset << " ";

    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(8);
    out_file << exp_profile_->get_error(i)  << std::endl;
  }
  out_file.close();

  // we are writing to the second file with .fit extension in the different
  // column order. the goal is to retire .dat extension after some time
  std::string file_name2 = file_name.substr(0, file_name.length()-4);
  file_name2 += ".fit";
  std::ofstream out_file2(file_name2.c_str());
  if (!out_file2) {
    IMP_THROW("Can't open file " << file_name2, IOException);
  }

  // header line
  out_file2.precision(15);
  out_file2 << "# SAXS profile: number of points = " << profile_size
            << ", q_min = " << exp_profile_->get_min_q()
            << ", q_max = " << exp_profile_->get_max_q();
  out_file2 << ", delta_q = " << exp_profile_->get_delta_q() << std::endl;

  out_file2.setf(std::ios::showpoint);
  out_file2 << "# offset = " << offset << ", scaling c = " << c
            << ", Chi^2 = " << score << std::endl;
  out_file2 << "#  q       exp_intensity   error model_intensity" << std::endl;

  out_file2.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_size; i++) {
    out_file2.setf(std::ios::left);
    out_file2.width(10);
    out_file2.precision(8);
    out_file2 << exp_profile_->get_q(i) << " ";

    out_file2.setf(std::ios::left);
    out_file2.width(15);
    out_file2.precision(8);
    out_file2 << exp_profile_->get_intensity(i) << " ";

    out_file2.setf(std::ios::left);
    out_file2.width(10);
    out_file2.precision(8);
    out_file2 << exp_profile_->get_error(i)  << " ";

    out_file2.setf(std::ios::left);
    out_file2.width(15);
    out_file2.precision(8);
    out_file2 << model_profile->get_intensity(i) * c - offset << std::endl;

  }
  out_file2.close();

}

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_FITTER_H */
