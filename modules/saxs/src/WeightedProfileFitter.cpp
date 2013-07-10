#include <IMP/saxs/WeightedProfileFitter.h>

#include <IMP/saxs/internal/nnls.h>

IMPSAXS_BEGIN_NAMESPACE

WeightedProfileFitter::WeightedProfileFitter(const Profile& exp_profile):
  ProfileFitter(exp_profile),
  W_(exp_profile.size()), Wb_(exp_profile.size()) {

  for(unsigned int i=0; i<exp_profile_.size(); i++) {
    Wb_[i] = exp_profile_.get_intensity(i);
    W_[i] = 1.0/(exp_profile_.get_error(i)*exp_profile_.get_error(i));
  }

  Wb_ = W_ * Wb_;

}

Float WeightedProfileFitter::compute_score(
                         const std::vector<IMP::saxs::Profile *>& profiles,
                         const std::string fit_file_name) const {
  int m = profiles.size();
  int n = exp_profile_.size();

  // no need to compute weighted profile for ensemble of size 1
  if(m == 1) return ProfileFitter::compute_score(*profiles[0], false, fit_file_name);

  Matrix A(n,m); // for intensities
  for(int j=0; j<m; j++) {
    Profile resampled_profile(exp_profile_.get_min_q(),
                              exp_profile_.get_max_q(),
                              exp_profile_.get_delta_q());
    resample(*profiles[j], resampled_profile);
    for(int i=0; i<n; i++) {
      A[i][j] = resampled_profile.get_intensity(i);
    }
  }

  // compute weights
  Vector x = autoregn(W_*A, Wb_);

  // computed weighted profile
  IMP::saxs::Profile weighted_profile(exp_profile_.get_min_q(),
                                      exp_profile_.get_max_q(),
                                      exp_profile_.get_delta_q());

  std::vector<double> weights(m);
  double weights_sum = 0;
  for(int i=0; i<x.size(); i++) {
    weights[i] = x[i];
    std::cout << "w[" << i << "] = " << x[i] << ", ";
    weighted_profile.add(*profiles[i], weights[i]);
    weights_sum += x[i];
  }

  double chi = ProfileFitter::compute_score(weighted_profile, false, fit_file_name);
  std::cout << "Chi = " << chi << std::endl;

  double c =  ProfileFitter::compute_scale_factor(weighted_profile);
  for(int i=0; i<x.size(); i++) {
    std::cout << "w[" << i << "] = " << x[i]/weights_sum << ", ";
  }

  return chi;
}

FitParameters WeightedProfileFitter::fit_profile(
                            std::vector<IMP::saxs::Profile *>& partial_profiles,
                            float min_c1, float max_c1,
                            float min_c2, float max_c2,
                            const std::string fit_file_name) const {

  // compute chi value for default c1/c1 (remove?)
  float default_c1 = 1.0, default_c2 = 0.0;
  for(unsigned int i=0; i<partial_profiles.size(); i++)
    partial_profiles[i]->sum_partial_profiles(default_c1, default_c2,
                                              *partial_profiles[i]);
  float default_chi = compute_score(partial_profiles);

  FitParameters fp = search_fit_parameters(partial_profiles,
                                           min_c1, max_c1, min_c2, max_c2,
                                           std::numeric_limits<float>::max());
  float best_c1 = fp.get_c1();
  float best_c2 = fp.get_c2();
  fp.set_default_chi(default_chi);

  // compute a profile for best c1/c2 combination
  for(unsigned int i=0; i<partial_profiles.size(); i++)
    partial_profiles[i]->sum_partial_profiles(best_c1, best_c2,
                                              *partial_profiles[i]);
  compute_score(partial_profiles, fit_file_name); // do we need it?

  std::cout << " Chi = " << fp.get_chi() << " c1 = " << best_c1 << " c2 = "
            << best_c2 << " default chi = " << default_chi << std::endl;
  return fp;
}


FitParameters WeightedProfileFitter::search_fit_parameters(
                           std::vector<IMP::saxs::Profile *>& partial_profiles,
                           float min_c1, float max_c1,
                           float min_c2, float max_c2,
                           float old_chi) const {
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

  // c1 iteration
  float c1(min_c1);
  for(int i=0; i<=c1_cells; i++, c1+= delta_c1) {
    // c2 iteration
    float c2 = min_c2;
    for(int j=0; j<=c2_cells; j++, c2+= delta_c2) {
      // sum up the profiles for c1/c2 combo
      for(unsigned int k=0; k<partial_profiles.size(); k++)
        partial_profiles[k]->sum_partial_profiles(c1, c2, *partial_profiles[k]);
      //partial_profile.sum_partial_profiles(c1, c2, partial_profile);
      float curr_chi = compute_score(partial_profiles);
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
    return search_fit_parameters(partial_profiles,
                                 min_c1, max_c1, min_c2, max_c2,
                                 best_chi);
  }
  return FitParameters(best_chi, best_c1, best_c2);
}

IMPSAXS_END_NAMESPACE
