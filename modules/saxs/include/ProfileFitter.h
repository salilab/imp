/**
 * \file IMP/saxs/ProfileFitter.h \brief a class for fitting two profiles
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_FITTER_H
#define IMPSAXS_PROFILE_FITTER_H

#include "ChiScore.h"
#include "FitParameters.h"
#include "Profile.h"

#include <fstream>

IMPSAXS_BEGIN_NAMESPACE

/** \name ProfileFitter

   ProfileFitter is a class for fitting the two profiles with user-defined
   scoring function that is a template parameter. By default chi score is used.
   The scoring function template parameter class has to implement three
   basic functions: compute_score, compute_scale_factor and compute_offset.
   see ChiScore for example.
   Currently two scoring functions are implemented: ChiScore and LogChiScore.
 */
template<class ScoringFunctionT = ChiScore>
class ProfileFitter: public base::RefCounted  {
public:
  //! Constructor
  /**
     \param[in] exp_profile Experimental profile we want to fit
  */
  ProfileFitter(const Profile& exp_profile): exp_profile_(exp_profile) {
    scoring_function_ = new ScoringFunctionT();
  }

  ProfileFitter(const Profile& exp_profile,
                ScoringFunctionT* sf): exp_profile_(exp_profile) {
    scoring_function_ = sf;
  }

  //! compute fit score
  Float compute_score(const Profile& model_profile,
                      bool use_offset = false,
                      const std::string fit_file_name = "") const;

  //! compute fit score in the interval
  Float compute_score(const Profile& model_profile,
                      Float min_q, Float max_q) const;

  //! fit experimental profile through optimization of c1 and c2 parameters
  /**
     \param[in] partial_profile  partial profiles computed
     \param[in] min/max c1, min/max c2 - search range for c1 and c2
     c1 - adjusts the excluded volume, valid range [0.95 - 1.05]
     c2 - adjusts the density of hydration layer, valid range [-2.0 - 4.0]
     \return FitParameters (score, c1, c2)
  */
  FitParameters fit_profile(Profile& partial_profile,
                            float min_c1=0.95, float max_c1=1.05,
                            float min_c2=-2.0, float max_c2=4.0,
                            bool use_offset = false,
                            const std::string fit_file_name = "") const;

  //! computes the scaling factor needed for fitting the modeled profile
  // onto the experimental one
  /** resampling of the modeled profile is required prior to calling
      to this function, in order to match the q values of the exp. profile.
      this is not done by default to minimize the number of calls to resample.
   */
  Float compute_scale_factor(const Profile& model_profile,
                             Float offset = 0.0) const {
    return scoring_function_->compute_scale_factor(exp_profile_, model_profile,
                                                   offset);
  }

  //! computes offset for fitting for fitting the modeled profile
  // onto the experimental one
  /** resampling of the modeled profile is required prior to calling
      to this function, in order to match the q values of the exp. profile.
      this is not done by default to minimize the number of calls to resample.
  */
  Float compute_offset(const Profile& model_profile) const {
    return scoring_function_->compute_offset(exp_profile_, model_profile);
  }

  //! resampling of the modeled profile is required to fit the q
  // values of the computational profile to the experimental one
  void resample(const Profile& model_profile, Profile& resampled_profile) const;

  // writes 3 column fit file, given scale factor c, offset and chi square
  void write_SAXS_fit_file(const std::string& file_name,
                           const Profile& model_profile,
                           const Float chi_square,
                           const Float c=1, const Float offset=0) const;

 private:
  FitParameters search_fit_parameters(Profile& partial_profile,
                                      float min_c1, float max_c1,
                                      float min_c2, float max_c2,
                                      bool use_offset, float old_chi) const;


  IMP_REF_COUNTED_DESTRUCTOR(ProfileFitter);
  friend class DerivativeCalculator;

 private:
  const Profile exp_profile_;   //  experimental saxs profile
  ScoringFunctionT* scoring_function_;
};

template<class ScoringFunctionT>
void ProfileFitter<ScoringFunctionT>::resample(const Profile& model_profile,
                                               Profile& resampled_profile) const
{
  // map of q values for fast search
  std::map<float, unsigned int> q_mapping;
  for (unsigned int k=0; k<model_profile.size(); k++) {
    q_mapping[model_profile.get_q(k)] = k;
  }

  for (unsigned int k=0; k<exp_profile_.size(); k++) {
    Float q = exp_profile_.get_q(k);
    if(q>model_profile.get_max_q()) break;
    std::map<float, unsigned int>::iterator it = q_mapping.lower_bound(q);
    if(it == q_mapping.end()) break;
    unsigned int i = it->second;
    if(i == 0) {
      resampled_profile.add_entry(q, model_profile.get_intensity(i));
    } else {
      Float delta_q = model_profile.get_q(i)-model_profile.get_q(i-1);
      if(delta_q <= 1.0e-16) {
        resampled_profile.add_entry(q, model_profile.get_intensity(i));
      } else {
        Float alpha = (q - model_profile.get_q(i-1)) / delta_q;
        if(alpha > 1.0) alpha = 1.0; // handle rounding errors
        Float intensity = model_profile.get_intensity(i-1)
          + (alpha)*(model_profile.get_intensity(i)
                     - model_profile.get_intensity(i-1));
        resampled_profile.add_entry(q, intensity);
      }
    }
  }
}

template<class ScoringFunctionT>
FitParameters ProfileFitter<ScoringFunctionT>::search_fit_parameters(
                                           Profile& partial_profile,
                                           float min_c1, float max_c1,
                                           float min_c2, float max_c2,
                                           bool use_offset, float old_chi) const
{
  int c1_cells = 10;
  int c2_cells = 10;
  if(old_chi < (std::numeric_limits<float>::max()-1)) { // second iteration
    c1_cells = 5;
    c2_cells = 5;
  }

  float delta_c1 = (max_c1-min_c1)/c1_cells;
  float delta_c2 = (max_c2-min_c2)/c2_cells;

  bool last_c1 = false;
  bool last_c2 = false;
  if(delta_c1 < 0.0001) { c1_cells=1; delta_c1 = max_c1-min_c1; last_c1=true; }
  if(delta_c2 < 0.001) { c2_cells=1; delta_c2 = max_c2-min_c2; last_c2=true; }
  float best_c1(1.0), best_c2(0.0), best_chi(old_chi);
  bool best_set = false;

  float c1(min_c1);
  for(int i=0; i<=c1_cells; i++, c1+= delta_c1) {
    float c2 = min_c2;
    for(int j=0; j<=c2_cells; j++, c2+= delta_c2) {
      partial_profile.sum_partial_profiles(c1, c2, partial_profile);
      float curr_chi = compute_score(partial_profile, use_offset);
      if(!best_set || curr_chi < best_chi) {
        best_set = true;
        best_chi = curr_chi;
        best_c1 = c1;
        best_c2 = c2;
      }
    }
  }

  if(std::fabs(best_chi-old_chi) > 0.0001 &&
     (!(last_c1 && last_c2))) { //refine more
    min_c1 = std::max(best_c1-delta_c1, min_c1);
    max_c1 = std::min(best_c1+delta_c1, max_c1);
    min_c2 = std::max(best_c2-delta_c2, min_c2);
    max_c2 = std::min(best_c2+delta_c2, max_c2);
    return search_fit_parameters(partial_profile,
                                 min_c1, max_c1, min_c2, max_c2,
                                 use_offset, best_chi);
  }
  return FitParameters(best_chi, best_c1, best_c2);
}

template<class ScoringFunctionT>
FitParameters ProfileFitter<ScoringFunctionT>::fit_profile(
                                 Profile& partial_profile,
                                 float min_c1, float max_c1,
                                 float min_c2, float max_c2,
                                 bool use_offset,
                                 const std::string fit_file_name) const {

  // compute chi value for default c1/c1 (remove?)
  float default_c1 = 1.0, default_c2 = 0.0;
  partial_profile.sum_partial_profiles(default_c1, default_c2, partial_profile);
  float default_chi = compute_score(partial_profile, use_offset);

  FitParameters fp = search_fit_parameters(partial_profile,
                                           min_c1, max_c1, min_c2, max_c2,
                                           use_offset,
                                           std::numeric_limits<float>::max());
  float best_c1 = fp.get_c1();
  float best_c2 = fp.get_c2();
  fp.set_default_chi(default_chi);

  // compute a profile for best c1/c2 combination
  partial_profile.sum_partial_profiles(best_c1, best_c2, partial_profile);
  compute_score(partial_profile, use_offset, fit_file_name);

  // std::cout << " Chi = " << best_chi << " c1 = " << best_c1 << " c2 = "
  //          << best_c2 << " default chi = " << default_chi << std::endl;
  return fp;
}

template<class ScoringFunctionT>
Float ProfileFitter<ScoringFunctionT>::compute_score(
                           const Profile& model_profile,
                           Float min_q, Float max_q) const
{
  Profile resampled_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  resample(model_profile, resampled_profile);

  Float score = scoring_function_->compute_score(exp_profile_,
                                                 resampled_profile,
                                                 min_q, max_q);
  return score;
}

template<class ScoringFunctionT>
Float ProfileFitter<ScoringFunctionT>::compute_score(
                           const Profile& model_profile,
                           bool use_offset,
                           const std::string fit_file_name) const
{
  Profile resampled_profile(exp_profile_.get_min_q(),
                            exp_profile_.get_max_q(),
                            exp_profile_.get_delta_q());
  resample(model_profile, resampled_profile);

  Float score = scoring_function_->compute_score(exp_profile_,
                                                 resampled_profile, use_offset);

  if(fit_file_name.length() > 0) {
    Float offset = 0.0;
    if(use_offset)
      offset=scoring_function_->compute_offset(exp_profile_, resampled_profile);
    Float c= scoring_function_->compute_scale_factor(exp_profile_,
                                                     resampled_profile, offset);
    write_SAXS_fit_file(fit_file_name, resampled_profile, score, c, offset);
  }
  return score;
}

template<class ScoringFunctionT>
void ProfileFitter<ScoringFunctionT>::write_SAXS_fit_file(
                                        const std::string& file_name,
                                        const Profile& model_profile,
                                        const Float score,
                                        const Float c,
                                        const Float offset) const {
  std::ofstream out_file(file_name.c_str());
  if (!out_file) {
    IMP_THROW("Can't open file " << file_name,
              IOException);
  }

  unsigned int profile_size = std::min(model_profile.size(),
                                       exp_profile_.size());
  // header line
  out_file.precision(15);
  out_file << "# SAXS profile: number of points = " << profile_size
           << ", q_min = " << exp_profile_.get_min_q()
           << ", q_max = " << exp_profile_.get_max_q();
  out_file << ", delta_q = " << exp_profile_.get_delta_q() << std::endl;

  out_file.setf(std::ios::showpoint);
  out_file << "# offset = " << offset << ", scaling c = " << c
           << ", Chi = " << score << std::endl;
  out_file << "#  q       exp_intensity   model_intensity"
           << std::endl;

  out_file.setf(std::ios::fixed, std::ios::floatfield);
  // Main data
  for (unsigned int i = 0; i < profile_size; i++) {
    out_file.setf(std::ios::left);
    out_file.width(10);
    out_file.precision(5);
    out_file << exp_profile_.get_q(i) << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << exp_profile_.get_intensity(i)  << " ";

    out_file.setf(std::ios::left);
    out_file.width(15);
    out_file.precision(8);
    out_file << model_profile.get_intensity(i)*c - offset << std::endl;
  }
  out_file.close();
}

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_FITTER_H */
