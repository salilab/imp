/**
 * \file IMP/saxs/ProfileClustering.h
 * \brief A class for profile clustering
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_CLUSTERING_H
#define IMPSAXS_PROFILE_CLUSTERING_H

#include <IMP/saxs/Profile.h>

IMPSAXS_BEGIN_NAMESPACE

/** Class for profile clustering */
class IMPSAXSEXPORT ProfileClustering {
public:
  ProfileClustering(Profile* exp_profile,
                    const Profiles& profiles,
                    double chi_percentage = 0.3, double chi_threshold = 0.0);

  ProfileClustering(Profile* exp_profile,
                    const Profiles& profiles,
                    const Vector<double>& scores,
                    double chi_percentage = 0.3, double chi_threshold = 0.0);

  const Vector<Profiles>& get_clusters() const {
    return clusters_;
  }

  const Profiles& get_representatives() const {
    return clustered_profiles_;
  }

private:
  void cluster_profiles();
  void select_representatives();

private:
  PointerMember<const Profile> exp_profile_;
  const Profiles profiles_;
  const Vector<double> scores_;

  Vector<double> chi_scores_;
  Profiles clustered_profiles_;
  Vector<Profiles> clusters_;
  double chi_percentage_;
  double chi_threshold_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_CLUSTERING_H */
