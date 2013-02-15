/**
 *  \file fitting_clustering.cpp
 *  \brief Cluster fitting solutions by rmsd
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/fitting_clustering.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/multifit/RMSDClustering.h>
#include <IMP/base_types.h>
IMPMULTIFIT_BEGIN_NAMESPACE

class FittingTransformation {
private:
  algebra::Transformation3D representative_rt_;
  algebra::Transformation3Ds all_rt_;
  int orig_index_;
  float cc_score_;
public:
  FittingTransformation(const algebra::Transformation3D &t,
                        float cc_score,int orig_index) {
    representative_rt_=t;
    all_rt_.push_back(t);
    cc_score_=cc_score;
    orig_index_ = orig_index;
  }
  void join_into(const FittingTransformation& t) {
    //todo - improve using average quaterion
    //if the new transformation (t) fits the map better, use it
    if (t.get_score() > cc_score_) {
      cc_score_ = t.get_score();
      orig_index_ = t.orig_index_;
      representative_rt_ = t.representative_rt_;
    }
    all_rt_.push_back(t.get_representative_transformation());
  }
  unsigned int get_number_of_transformations() const {
    return all_rt_.size();
  }
  void update_score(float ) {
    //score_ += s;
  }
  algebra::Transformation3D get_representative_transformation() const {
    return representative_rt_;
  }
  float get_score() const {return cc_score_;}
  algebra::Transformation3D get_member_transformation(int i) const {
    return all_rt_[i];
  }
  int get_index() const {return orig_index_;}
};

em::FittingSolutions fitting_clustering (
    const atom::Hierarchy &mh,
    const em::FittingSolutions &ts,
    float spacing, int top_sols,float rmsd) {

  //translate imp tranformations to gamb transformations
  //and prepare for clustering
  std::vector<FittingTransformation> clustered_ts_temp;
  std::vector<FittingTransformation> f_ts;
  for(int i=0;i<std::min(top_sols,ts.get_number_of_solutions());i++) {
    f_ts.push_back(FittingTransformation(ts.get_transformation(i),
                                         ts.get_score(i),i));
  }
  IMP_LOG_TERSE("going to cluster : " << f_ts.size()
          << " transformations for protein : " << std::endl);
  //do the clustering
  multifit::RMSDClustering<FittingTransformation> clusterer;
  clusterer.prepare(core::get_leaves(mh));
  clusterer.set_bin_size(2.0*spacing);
  clusterer.cluster(rmsd,f_ts,clustered_ts_temp);
  //set output
  em::FittingSolutions clustered_ts;
  for(unsigned int i=0;i<clustered_ts_temp.size();i++) {
    clustered_ts.add_solution(
                    clustered_ts_temp[i].get_representative_transformation(),
                              clustered_ts_temp[i].get_score());
  }
  IMP_LOG_TERSE("number of clustered transformations "
          << clustered_ts.get_number_of_solutions() << std::endl);
  return clustered_ts;
}
IMPMULTIFIT_END_NAMESPACE
