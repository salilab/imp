/**
 *  \file TransformationClustering.cpp
 *  \brief Clustering of transformation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/TransformationClustering.h>
#include <IMP/multifit/GeometricHash.h>
#include <IMP/multifit/RMSDClustering.h>
IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
  class TransformationWrapper {
  private:
    algebra::Transformation3D representative_rt_;
    algebra::Transformation3Ds all_rt_;
    int orig_index_;
  public:
    TransformationWrapper(const algebra::Transformation3D &t,
                          int orig_index) {
    representative_rt_=t;
    all_rt_.push_back(t);
    orig_index_ = orig_index;
  }
  void join_into(const TransformationWrapper& t) {
    //add all transformations
    all_rt_.insert(all_rt_.end(),t.all_rt_.begin(),t.all_rt_.end());
  }
    inline float get_score() const { return 1.;}
  unsigned int get_number_of_transformations() const {
    return all_rt_.size();
  }
  void update_score(float s) {
    //score_ += s;
  }
  algebra::Transformation3D get_representative_transformation() const {
    return representative_rt_;
  }
    algebra::Transformation3D get_member_transformation(int i) const {
      return all_rt_[i];
    }
    int get_index() const {return orig_index_;}
  };//end TransformationWrapper
  typedef std::vector<TransformationWrapper> TransformationWrappers;
  typedef std::pair<algebra::Rotation3D,int> RotationInd;
  typedef GeometricHash<RotationInd, 4> HashRot;
  typedef std::pair<algebra::Transformation3D,int> TransformationInd;
  typedef GeometricHash<TransformationInd, 7> HashTrans;
  TransformationWrappers wrap_transformations(
                        const algebra::Transformation3Ds &trans) {
    TransformationWrappers ret;
    for(int i=0;i<(int)trans.size();i++) {
      ret.push_back(TransformationWrapper(trans[i],i));
    }
    return ret;
  }
  algebra::Transformation3Ds unwrap_transformations(
                    const TransformationWrappers &tw,
                    int min_cluster_size) {
    algebra::Transformation3Ds ret;
    for(int i=0;i<(int)tw.size();i++) {
      IMP_LOG(VERBOSE,"Unwrapping cluster of "<<
              tw[i].get_number_of_transformations()<<std::endl);
      if(tw[i].get_number_of_transformations()<(unsigned int)min_cluster_size)
        continue;
      ret.push_back(tw[i].get_representative_transformation());
    }
    return ret;
  }
  }//end namespace
TransformationClustering::TransformationClustering(Particles ps,
                           Float max_rmsd) :
  max_rmsd_(max_rmsd) {
  ps_=ps;
  }
algebra::Transformation3Ds TransformationClustering::cluster_by_rmsd(
               const algebra::Transformation3Ds &trans,
               float max_rmsd,
               int min_cluster_size) {
    //create records
    TransformationWrappers tw = wrap_transformations(trans);
    TransformationWrappers twc;//transformations after clustering
    RMSDClustering<TransformationWrapper> engine;
    engine.set_bin_size(1);
    engine.prepare(ps_);
    engine.cluster(max_rmsd,tw,twc);
    IMP_LOG(VERBOSE,"After clsutering :"<<twc.size()<<" records \n");
    return unwrap_transformations(twc,min_cluster_size);
  }
//TODO - implement this function!
algebra::Transformation3Ds TransformationClustering::cluster_by_transformation(
               const algebra::Transformation3Ds &trans,
               float max_angle_diff_in_rad,float max_translation_diff,
               int min_cluster_size) {
    algebra::Transformation3Ds ret;
    algebra::VectorD<7> rad_vec;
    for(int i=0;i<4;i++){
      rad_vec[i]=max_angle_diff_in_rad;}
    for(int i=4;i<7;i++){
      rad_vec[i]=max_translation_diff;}
    //fill the hash
    HashTrans trans_hash(rad_vec);
    for(unsigned int i = 0; i < trans.size(); i++ ) {
      algebra::VectorD<7> trans_vec;
      for(int j=0;j<4;j++){
        trans_vec[j]=trans[i].get_rotation().get_quaternion()[j];}
      for(int j=4;j<7;j++){
        trans_vec[j]=trans[i].get_translation()[j-4];}
      trans_hash.add(trans_vec,
                     TransformationInd(trans[i],i));
    }

  // try to reduce ignore_clusters_smaller in case no
  //transformations pass the clustering stage
  const HashTrans::GeomMap &trans_map = trans_hash.Map();
  while (min_cluster_size > 0) {
    //cluster transformations in each bucket based on RMSD
    for (HashTrans::GeomMap::const_iterator bucket_iter = trans_map.begin();
          bucket_iter != trans_map.end(); ++bucket_iter ) {
      const HashTrans::PointList &bucket = bucket_iter->second;
      if(bucket.size() >= (unsigned int)min_cluster_size){
        //get the transformations in the bucket
        algebra::Transformation3Ds all_trans_in_bucket;
        for(int j=0;j<(int)bucket.size();j++) {
          TransformationInd trans_ind=bucket[j].second;
          all_trans_in_bucket.push_back(trans[trans_ind.second]);
        }
        //TODO - is this max_rmsd_ ok?
        algebra::Transformation3Ds clustered_trans_in_bucket=
          cluster_by_rmsd(all_trans_in_bucket,max_rmsd_,
                          min_cluster_size);
        ret.insert(ret.end(),clustered_trans_in_bucket.begin(),
                   clustered_trans_in_bucket.end());
      }
    }
    IMP_LOG(VERBOSE,"ret size:"<<ret.size()<<std::endl);
    if(ret.size() > 0) {
      break;
    } else { // reduce parameter
      min_cluster_size--;
      IMP_WARN("no clusters, reducing size"<<std::endl);
    }
  }
  IMP_LOG(VERBOSE,"return ret size:"<<ret.size()<<std::endl);
  return ret;
}
algebra::Transformation3Ds
  TransformationClustering::cluster_by_rotation(
                  const algebra::Transformation3Ds &trans,
                  float max_angle_diff_in_rad,
                  int min_cluster_size) {
  algebra::Transformation3Ds ret;
  std::cout<<"min_rot_cluster_size:"<<min_cluster_size<<std::endl;
  // create GeomHash with transforms hashed according
  //to 4 quaternion rotation parameters
  algebra::VectorD<4> rad_vec;
  for(int i=0;i<4;i++){
    rad_vec[i]=max_angle_diff_in_rad;}
  HashRot rot_hash(rad_vec);
  for(unsigned int i = 0; i < trans.size(); i++ ) {
    rot_hash.add(
                 trans[i].get_rotation().get_quaternion(),
                 RotationInd(trans[i].get_rotation(),i));
  }

  // try to reduce ignore_clusters_smaller in case no
  //transformations pass the clustering stage
  const HashRot::GeomMap &rot_map = rot_hash.Map();
  while (min_cluster_size > 0) {
    IMP_LOG(TERSE,"min_cluster_size:"<<min_cluster_size<<std::endl);
    //cluster transformations in each bucket based on RMSD
    for (HashRot::GeomMap::const_iterator bucket_iter = rot_map.begin();
          bucket_iter != rot_map.end(); ++bucket_iter ) {
      const HashRot::PointList &bucket = bucket_iter->second;
      if(bucket.size() >= (unsigned int)min_cluster_size){
        //get the transformations in the bucket
        algebra::Transformation3Ds all_trans_in_bucket;
        for(int j=0;j<(int)bucket.size();j++) {
          RotationInd rot_ind=bucket[j].second;
          all_trans_in_bucket.push_back(trans[rot_ind.second]);
        }
        IMP_LOG(VERBOSE,
           "RMSD of bucket of size:"<<all_trans_in_bucket.size()<<std::endl);
        algebra::Transformation3Ds clustered_trans_in_bucket=
          cluster_by_rmsd(all_trans_in_bucket,max_rmsd_,
                          min_cluster_size);
        ret.insert(ret.end(),clustered_trans_in_bucket.begin(),
                   clustered_trans_in_bucket.end());
      }
    }
    if(ret.size() > 0) {
      break;
    } else { // reduce parameter
      min_cluster_size--;
      std::cout<<"no clusters, reducing size"<<std::endl;
    }
  }

  return ret;
}
algebra::Transformation3Ds get_clustered(Particles ps,
                               const algebra::Transformation3Ds &trans,
                               float max_rmsd,
                               float  max_angle_diff_in_rad,
                               float max_displace,
                               int min_cluster_size) {
  // std::cout<<
  //   "running ... Clustering of "<<trans.size()<< std::endl;

  IMP_LOG(TERSE,
          "running ... Clustering of "<<trans.size()<< std::endl);

  if (trans.size()==0) {
    algebra::Transformation3Ds ret;
    return ret;
  }
  TransformationClustering fast_clust(ps, max_rmsd);
  algebra::Transformation3Ds clust_trans_by_rot =
    fast_clust.cluster_by_rotation(trans,max_angle_diff_in_rad,
                                   min_cluster_size);
  std::cout<<"done clustering by rotation from:"<<trans.size()
           <<" to "<<clust_trans_by_rot.size()<<std::endl;
  IMP_LOG(TERSE,"done clustering by rotation from:"<<trans.size()
          <<" to "<<clust_trans_by_rot.size()<<std::endl);

  algebra::Transformation3Ds clust_trans_by_trans =
    fast_clust.cluster_by_transformation(clust_trans_by_rot,
                                         max_angle_diff_in_rad,
                                         max_displace,
                                         min_cluster_size);
  std::cout<<"done clustering by transformation from:"
           <<clust_trans_by_rot.size()<<" to "
           << clust_trans_by_trans.size()<<std::endl;
  IMP_LOG(TERSE,"done clustering by transformation from:"
          <<clust_trans_by_rot.size()<<" to "
          << clust_trans_by_trans.size()<<std::endl);

  algebra::Transformation3Ds clust_trans =
    fast_clust.cluster_by_rmsd(clust_trans_by_trans,max_rmsd,
                               min_cluster_size);
  std::cout<<"done clustering by rmsd from:"
           <<clust_trans_by_trans.size()<<" to: "
           <<clust_trans.size()<<std::endl;
  IMP_LOG(TERSE,"done clustering by rmsd from:"
          <<clust_trans_by_trans.size()<<" to: "
          <<clust_trans.size()<<std::endl);
  return clust_trans;
}

IMPMULTIFIT_END_NAMESPACE
