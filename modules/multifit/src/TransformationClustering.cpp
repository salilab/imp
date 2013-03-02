/**
 *  \file TransformationClustering.cpp
 *  \brief Clustering of transformation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/TransformationClustering.h>
#include <IMP/multifit/GeometricHash.h>
#include <IMP/multifit/RMSDClustering.h>
IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
  class TransformationIndex {
  public:
    TransformationIndex(const algebra::Transformation3D &t,
                        int i,double score) : t_(t),i_(i),score_(score){}
    TransformationIndex(const algebra::Transformation3D &t,
                        int i) : t_(t),i_(i),score_(0.){}
    algebra::Transformation3D t_;
    int i_;
    double score_;
  };
  typedef std::vector<TransformationIndex> TransformationIndexes;
  class TransformationWrapper {
  private:
    TransformationIndex representative_rt_;
    TransformationIndexes members_;
  public:
#if 0
    TransformationWrapper(const algebra::Transformation3D &t,
                          int orig_index) : representative_rt_(t,orig_index,0)
    {}
#endif
    TransformationWrapper(const algebra::Transformation3D &t,
                          int orig_index,double score) :
      representative_rt_(t,orig_index,score)
    {}
    void join_into(const TransformationWrapper& t) {
    //add all transformations
    members_.insert(members_.end(),t.members_.begin(),t.members_.end());
    members_.push_back(t.representative_rt_);
    }
    inline float get_score() const { return representative_rt_.score_;}
    unsigned int get_number_of_transformations() const {
      return members_.size()+1;
    }
    void update_score(float /*s*/) {
      //score_ += s;
    }
    Ints get_all_indexes() const {
      Ints ret(members_.size()+1);
      ret[0]=representative_rt_.i_;
      for (int i=0;i<(int)members_.size();i++) {
        ret[i+1]=members_[i].i_;
      }
      return ret;
    }
    algebra::Transformation3D get_representative_transformation() const {
      return representative_rt_.t_;
    }
#if 0
    algebra::Transformation3D get_member_transformation(int i) const {
      return members_[i].t_;
    }
#endif
    //    int get_index() const {return orig_index_;}
  };//end TransformationWrapper
  typedef std::vector<TransformationWrapper> TransformationWrappers;
  typedef std::pair<algebra::Rotation3D,int> RotationInd;
  typedef GeometricHash<RotationInd, 4> HashRot;
  //typedef std::pair<algebra::Transformation3D,int> TransformationInd;
  typedef GeometricHash<TransformationIndex, 7> HashTrans;
  TransformationWrappers wrap_transformations(
                        const algebra::Transformation3Ds &trans) {
    TransformationWrappers ret;
    for(int i=0;i<(int)trans.size();i++) {
      ret.push_back(TransformationWrapper(trans[i],i,0));
    }
    return ret;
  }
#if 0
  TransformationWrappers wrap_transformations(
                                        const em::FittingSolutions &fits) {
    TransformationWrappers ret;
    for(int i=0;i<(int)fits.get_number_of_solutions();i++) {
      ret.push_back(TransformationWrapper(
       fits.get_transformation(i),i,fits.get_score(i)));
    }
    return ret;
  }
#endif
  IntsList unwrap_transformations(
                    const TransformationWrappers &tw,
                    int min_cluster_size) {
    IntsList ret;
    for(int i=0;i<(int)tw.size();i++) {
      IMP_LOG_VERBOSE("Unwrapping cluster of "<<
              tw[i].get_number_of_transformations()<<std::endl);
      if(tw[i].get_number_of_transformations()<(unsigned int)min_cluster_size)
        continue;
      ret.push_back(tw[i].get_all_indexes());
    }
    return ret;
  }
  }//end namespace

TransformationClustering::TransformationClustering(Particles ps,
                           Float max_rmsd) :
  base::Object("TransformationClustering%1%"), max_rmsd_(max_rmsd) {
  ps_=ps;
  }
IntsList TransformationClustering::cluster_by_rmsd(
               const algebra::Transformation3Ds &trans,
               float max_rmsd,
               int min_cluster_size) {
    //create records
    TransformationWrappers tw = wrap_transformations(trans);
    TransformationWrappers twc;//transformations after clustering
    IMP::multifit::RMSDClustering<TransformationWrapper> engine;
    engine.set_bin_size(1);
    engine.prepare(get_as<ParticlesTemp>(ps_));
    engine.cluster(max_rmsd,tw,twc);
    IMP_LOG_VERBOSE("After clustering :"<<twc.size()<<" records \n");
    return unwrap_transformations(twc,min_cluster_size);
  }

IntsList TransformationClustering::cluster_by_transformation(
               const algebra::Transformation3Ds &trans,
               float max_angle_diff_in_rad,float max_translation_diff,
               int min_cluster_size) {
  IntsList ret;
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
                   TransformationIndex(trans[i],i));
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
        std::map<int,int> ind_map;
        for(int j=0;j<(int)bucket.size();j++) {
          TransformationIndex trans_ind=bucket[j].second;
          all_trans_in_bucket.push_back(trans_ind.t_);
          ind_map[j]=trans_ind.i_;
        }
        //TODO - is this max_rmsd_ ok?
        IntsList clustered_trans_in_bucket=
          cluster_by_rmsd(all_trans_in_bucket,max_rmsd_,
                          min_cluster_size);
        //retrieve original indexes
        for(IntsList::const_iterator it =
              clustered_trans_in_bucket.begin();
            it != clustered_trans_in_bucket.end();it++) {
          Ints correct_inds(it->size());
          for(int l=0;l<(int)it->size();l++) {
            correct_inds[l]=ind_map[(*it)[l]];
          }
          ret.push_back(correct_inds);
        }//end retrieve orig inds
      }
    }//end iterate bucket
    IMP_LOG_VERBOSE("ret size:"<<ret.size()<<std::endl);
    if(ret.size() > 0) {
      break;
    } else { // reduce parameter
      min_cluster_size--;
      IMP_WARN("no clusters, reducing size"<<std::endl);
    }
  }
  IMP_LOG_VERBOSE("return ret size:"<<ret.size()<<std::endl);
  return ret;
}
IntsList
  TransformationClustering::cluster_by_rotation(
                  const algebra::Transformation3Ds &trans,
                  float max_angle_diff_in_rad,
                  int min_cluster_size) {
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
  IntsList ret;
  // try to reduce ignore_clusters_smaller in case no
  //transformations pass the clustering stage
  const HashRot::GeomMap &rot_map = rot_hash.Map();
  while (min_cluster_size > 0) {
    IMP_LOG_TERSE("min_cluster_size:"<<min_cluster_size<<std::endl);
    //cluster transformations in each bucket based on RMSD
    for (HashRot::GeomMap::const_iterator bucket_iter = rot_map.begin();
          bucket_iter != rot_map.end(); ++bucket_iter ) {
      const HashRot::PointList &bucket = bucket_iter->second;
      if(bucket.size() >= (unsigned int)min_cluster_size){
        //get the transformations in the bucket
        algebra::Transformation3Ds all_trans_in_bucket;
        std::map<int,int> rot_ind2_orig_ind;
        for(int j=0;j<(int)bucket.size();j++) {
          RotationInd rot_ind=bucket[j].second;
          all_trans_in_bucket.push_back(trans[rot_ind.second]);
          rot_ind2_orig_ind[j]=rot_ind.second;
        }
        IMP_LOG_VERBOSE(
           "RMSD of bucket of size:"<<all_trans_in_bucket.size()<<std::endl);
        IntsList clustered_trans_in_bucket=
          cluster_by_rmsd(all_trans_in_bucket,max_rmsd_,
                          min_cluster_size);
        //retrieve orig inds
        for(IntsList::const_iterator it =
              clustered_trans_in_bucket.begin();
            it != clustered_trans_in_bucket.end();it++) {
          Ints inds(it->size());
          for(int j=0;j<(int)it->size();j++) {
            inds[j]=rot_ind2_orig_ind[(*it)[j]];
          }
          ret.push_back(inds);
        }
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
IntsList get_clustered(Particles ps,
                       const algebra::Transformation3Ds &trans,
                       float max_rmsd,
                       float  max_angle_diff_in_rad,
                       float max_displace,
                       int min_cluster_size) {
  // std::cout<<
  //   "running ... Clustering of "<<trans.size()<< std::endl;

  IMP_LOG_TERSE(
          "running ... Clustering of "<<trans.size()<< std::endl);

  if (trans.size()==0) {
    IntsList ret;
    return ret;
  }
  IMP_NEW(TransformationClustering, fast_clust, (ps, max_rmsd));
  IntsList clust_trans_by_rot_inds =
    fast_clust->cluster_by_rotation(trans,max_angle_diff_in_rad,
                                    min_cluster_size);
  algebra::Transformation3Ds clust_trans_by_rot(clust_trans_by_rot_inds.size());
  std::map<int,int> rot_ind2orig_ind;
  for(int i=0;i<(int)clust_trans_by_rot_inds.size();i++) {
    clust_trans_by_rot[i]=trans[clust_trans_by_rot_inds[i][0]];
    rot_ind2orig_ind[i]=clust_trans_by_rot_inds[i][0];
  }
  std::cout<<"done clustering by rotation from:"<<trans.size()
           <<" to "<<clust_trans_by_rot.size()<<std::endl;
  IMP_LOG_TERSE("done clustering by rotation from:"<<trans.size()
          <<" to "<<clust_trans_by_rot.size()<<std::endl);

  IntsList clust_trans_by_trans_inds =
    fast_clust->cluster_by_transformation(clust_trans_by_rot,
                                          max_angle_diff_in_rad,
                                          max_displace, min_cluster_size);
  //retrieve the transformations and the original indexes
  algebra::Transformation3Ds clust_trans_by_trans(
                                        clust_trans_by_trans_inds.size());
  std::map<int,int> trans_ind2orig_ind;
  for(int i=0;i<(int)clust_trans_by_trans_inds.size();i++) {
    trans_ind2orig_ind[i]=rot_ind2orig_ind[clust_trans_by_trans_inds[i][0]];
    clust_trans_by_trans[i]=clust_trans_by_rot[clust_trans_by_trans_inds[i][0]];
  }
  std::cout<<"done clustering by transformation from:"
           <<clust_trans_by_rot.size()<<" to "
           << clust_trans_by_trans.size()<<std::endl;
  IMP_LOG_TERSE("done clustering by transformation from:"
          <<clust_trans_by_rot.size()<<" to "
          << clust_trans_by_trans.size()<<std::endl);

  IntsList clust_trans_inds=
    fast_clust->cluster_by_rmsd(clust_trans_by_trans,max_rmsd,
                                min_cluster_size);

  IntsList ret(clust_trans_inds.size());
  for(int i=0;i<(int)clust_trans_inds.size();i++) {
    Ints orig_inds(clust_trans_inds[i].size());
    for(int j=0;j<(int)clust_trans_inds[i].size();j++) {
      orig_inds[j]=trans_ind2orig_ind[clust_trans_inds[i][j]];
    }
    ret[i]=orig_inds;
  }

  std::cout<<"done clustering by rmsd from:"
           <<clust_trans_by_trans.size()<<" to: "
           <<ret.size()<<std::endl;
  IMP_LOG_TERSE("done clustering by rmsd from:"
          <<clust_trans_by_trans.size()<<" to: "
          <<ret.size()<<std::endl);
  return ret;
}

IMPMULTIFIT_END_NAMESPACE
