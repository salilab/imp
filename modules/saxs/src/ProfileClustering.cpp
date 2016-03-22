/**
 *  \file ProfileClustering.cpp
 *
 * \brief A class for profiles clustering
 *
 * Author: Dina Schneidman
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/saxs/ProfileClustering.h>

#include <IMP/saxs/ChiScore.h>
#include <IMP/saxs/ProfileFitter.h>

IMPSAXS_BEGIN_NAMESPACE

ProfileClustering::ProfileClustering(Profile* exp_profile,
                                     const Profiles& profiles,
                                     double chi_percentage, double chi_threshold) :
  exp_profile_(exp_profile),
  profiles_(profiles),
  chi_percentage_(chi_percentage),
  chi_threshold_(chi_threshold)
{
  cluster_profiles();
}

ProfileClustering::ProfileClustering(Profile* exp_profile,
                                     const Profiles& profiles,
                                     const Vector<double>& scores,
                                     double chi_percentage, double chi_threshold) :
    exp_profile_(exp_profile),
    profiles_(profiles),
    scores_(scores),
    chi_percentage_(chi_percentage),
    chi_threshold_(chi_threshold)
{
  if(chi_percentage_ > 0.00001) cluster_profiles();
}

void ProfileClustering::cluster_profiles() {

  // compute Chi values and copy errors
  std::multimap<double, int> scored_profiles;
  Pointer<ProfileFitter<ChiScore> > pf = new ProfileFitter<ChiScore>(exp_profile_);
  IMP_NEW(ChiScore, chi_score, ());
  chi_score->set_was_used(true);
  Profiles resampled_profiles(profiles_.size());
  for(unsigned int i=0; i<profiles_.size(); i++) {
    // resample all models profiles
    Profile *resampled_profile = new Profile(exp_profile_->get_min_q(),
                                             exp_profile_->get_max_q(),
                                             exp_profile_->get_delta_q());
    profiles_[i]->resample(exp_profile_, resampled_profile);
    FitParameters fp  = pf->fit_profile(resampled_profile);
    scored_profiles.insert(std::make_pair(fp.get_chi(), i));
    chi_scores_.push_back(fp.get_chi());
    resampled_profile->copy_errors(exp_profile_);
    resampled_profile->scale(chi_score->compute_scale_factor(exp_profile_,
                                                            resampled_profile));
    profiles_[i]->set_id(i);
    resampled_profiles[i] = resampled_profile;
  }

  // cluster
  clusters_.reserve(profiles_.size()/4); // approximately
  std::multimap<double, int> &temp_profiles(scored_profiles);
  double threshold = chi_percentage_ * (temp_profiles.begin()->first);
  if(chi_threshold_ > 0.0) threshold = chi_threshold_;
  std::cout  << "clustering threshold = " << threshold << std::endl;
  int cluster_number = 1;
  while(!temp_profiles.empty()) {
    int cluster_profile_id = temp_profiles.begin()->second;
    Profiles curr_cluster;
    Profile *cluster_profile = resampled_profiles[cluster_profile_id];
    curr_cluster.push_back(profiles_[cluster_profile_id]);
    // remove first
    temp_profiles.erase(temp_profiles.begin());

    std::multimap<double, int>::iterator it = temp_profiles.begin();
    // iterate over the rest of the profiles and erase similar ones
    while(it != temp_profiles.end()) {
      int curr_profile_id = it->second;
      Profile *curr_profile = resampled_profiles[curr_profile_id];
      double score = chi_score->compute_score(cluster_profile, curr_profile);
      if(score < threshold) {
        curr_cluster.push_back(profiles_[curr_profile_id]);
        temp_profiles.erase(it++);
      } else {
        it++;
      }
    }
    clusters_.push_back(curr_cluster);
    cluster_number++;
  }

  std::cout << "Number of clusters = " << cluster_number-1 << std::endl;
  select_representatives();
}

void ProfileClustering::select_representatives() {
  bool select_by_chi = true;
  if(scores_.size() == profiles_.size()) select_by_chi = false;

  clustered_profiles_.resize(clusters_.size());
  for(unsigned int i=0; i<clusters_.size(); i++) {
    if(select_by_chi) clustered_profiles_[i] = clusters_[i][0];
    else { // select best scoring
      unsigned int best_member = 0;
      double best_score = scores_[clusters_[i][0]->get_id()];
      for(unsigned int j=1; j<clusters_[i].size(); j++) {
        if(scores_[clusters_[i][j]->get_id()] <  best_score) {
          best_score = scores_[clusters_[i][j]->get_id()];
          best_member = j;
        }
      }
      clustered_profiles_[i] = clusters_[i][best_member];
    }
  }
}

IMPSAXS_END_NAMESPACE
