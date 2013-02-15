/**
 *  \file IMP/multifit/RMSDClustering.h
 *  \brief Cluster transformations by rmsd
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_RMSD_CLUSTERING_H
#define IMPMULTIFIT_RMSD_CLUSTERING_H

#include <IMP/multifit/multifit_config.h>
#include "GeometricHash.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/core/XYZ.h>
#include <boost/graph/adjacency_list.hpp>
#include <IMP/atom/distance.h>
IMPMULTIFIT_BEGIN_NAMESPACE

//! RMSD clustering
/**
  /note Iteratively joins pairs of close transformations. The algorithm first
  clusters transformations for which the transformed centroids are close
  (fall into the same bin in a hash). Then, all clusters are globally
  reclustered.
  /note TransT should implement the functions:
        join_into() add a transformation to the current cluster and
                   possibly updates the representative transformation for the
                   cluster
        get_score() that returns the score (higher score is better)
        update_score() that updates the score of the
                    cluster according to a new member
        get_representative_transformation() a function that returns the
                    the representative transformation for a cluster

*/
template <class TransT>
class RMSDClustering {
private:
//! Base class for transformation record
class TransformationRecord {
public:
  ////standard constructor.
  inline TransformationRecord(const TransT &trans):
    valid_(true), trans_(trans) {
  }
  virtual ~TransformationRecord() {}
  //! Join the transformations into this.
  void join_into(const TransformationRecord& record) {
    trans_.update_score(record.trans_.get_score());
    trans_.join_into(record.trans_);
  }
  inline float get_score() const { return trans_.get_score();}
  const algebra::Vector3D get_centroid() const { return centroid_; }
  void set_centroid(algebra::Vector3D& centroid) {
    centroid_ = trans_.get_representative_transformation().get_transformed(
                                                                   centroid); }
  TransT get_record() const {return trans_;}
  bool get_valid() const {return valid_;}
  void set_valid(bool v) {valid_=v;}
protected:
  bool valid_;
  TransT trans_;
  algebra::Vector3D centroid_;
};
typedef std::vector<TransformationRecord> TransformationRecords;
public:
  typedef GeometricHash<int, 3> Hash3;
  typedef boost::property<boost::edge_weight_t, short> ClusEdgeWeightProperty;
  typedef boost::property<boost::vertex_index_t, int> ClusVertexIndexProperty;
  // Graph type
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                   ClusVertexIndexProperty, ClusEdgeWeightProperty> Graph;
  typedef boost::graph_traits<Graph> RCGTraits;
  typedef RCGTraits::vertex_descriptor RCVertex;
  typedef RCGTraits::edge_descriptor RCEdge;
  typedef RCGTraits::vertex_iterator RCVertexIt;
  typedef RCGTraits::edge_iterator RCEdgeIt;

  struct sort_by_weight {
    bool operator()(const std::pair<RCEdge,float> &s1,
                    const std::pair<RCEdge,float> &s2) const {
      return s1.second < s2.second;
    }
  };
  /**
     \param[in] bin_size the radius of the bins of the hash
     differ with at most this value
   */
  RMSDClustering(float bin_size=3.){is_ready_=false;bin_size_=bin_size;}
  virtual ~RMSDClustering() {}
  //! cluster transformations
  void cluster(float max_dist, const std::vector<TransT>& input_trans,
               std::vector<TransT>& output_trans);
  //! prepare for clustering
  void prepare(const ParticlesTemp &ps);
  void set_bin_size(float bin_size) {bin_size_=bin_size;}
protected:
  //! returns the RMSD between two transformations with respect to
  //! the stored points
  virtual float get_squared_distance(const TransT& trans1,
                                     const TransT& trans2);
  //clustering function
void build_graph(const Hash3::PointList &inds,
                 const std::vector<TransformationRecord*> &recs,
                 float max_dist,
                 Graph &g);

 void build_full_graph(const Hash3 &h,
                   const std::vector<TransformationRecord*> &recs,
                   float max_dist, Graph &g);

 int cluster_graph(Graph &g,
                   const std::vector<TransformationRecord*> &recs,
                   float max_dist);

 int fast_clustering(float max_dist,
                     std::vector<TransformationRecord *>& recs);

 virtual int exhaustive_clustering(float max_dist,
           std::vector<TransformationRecord *>& recs);
 //! Remove transformations which are not valid.
 // should be used after each invocation of work.
 virtual void clean(std::vector<TransformationRecord*>*& records);
 bool is_fast_;
 float bin_size_; //hash bin size
 // The centroid of the molecule
 algebra::Vector3D centroid_;
 Particles ps_;
 core::XYZs xyzs_;
 //fast RMSD computation
 atom::RMSDCalculator rmsd_calc_;
 bool is_ready_;
};

template<class TransT> float
RMSDClustering<TransT>::get_squared_distance(const TransT& trans1,
                                     const TransT& trans2) {
  return rmsd_calc_.get_squared_rmsd(trans1.get_representative_transformation(),
                                  trans2.get_representative_transformation());
}

template<class TransT>
void RMSDClustering<TransT>::build_graph(const Hash3::PointList &inds,
                   const std::vector<TransformationRecord*> &recs,
                                 float max_dist, Graph &g){
  //hash all the records
  float max_dist2=max_dist*max_dist;
  //add nodes
IMP_LOG_VERBOSE("build_graph:adding nodes"<<std::endl);
  std::vector<RCVertex> nodes(inds.size());
  for (unsigned int i=0; i<inds.size(); ++i) {
    nodes[i]=boost::add_vertex(i,g);
  }
  //add edges
  IMP_LOG_VERBOSE("build_graph:adding edges"<<std::endl);
  for (unsigned int i=0; i<inds.size(); ++i) {
    for (unsigned int j=i+1; j<inds.size(); ++j) {
      float d2 = get_squared_distance(recs[i]->get_record(),
                                      recs[j]->get_record());
      if (d2 < max_dist2) {
        boost::add_edge(nodes[i],nodes[j],d2,g);
        //edge_weight.push_back(std::pair<RCEdge,float>(e,d2));
      }}}
  IMP_LOG_VERBOSE("build_graph: done building"<<std::endl);
}
template<class TransT>
void RMSDClustering<TransT>::build_full_graph(const Hash3 &h,
                   const std::vector<TransformationRecord*> &recs,
                                 float max_dist, Graph &g){
  float max_dist2=max_dist*max_dist;
  //add nodes
  std::vector<RCVertex> nodes(recs.size());
  for (unsigned int i=0; i<recs.size(); ++i) {
    nodes[i]=boost::add_vertex(i,g);
  }
  //add edges
  for (int i = 0 ; i < (int)recs.size() ; ++i) {
    TransT tr=recs[i]->get_record();
    algebra::Transformation3D t = tr.get_representative_transformation();
    Hash3::HashResult result =
      h.neighbors(Hash3::INF, t.get_transformed(centroid_), max_dist);
    for ( size_t k=0; k<result.size(); ++k ) {
      int j = result[k]->second;
      if (i >= j) continue; //insert edge only once
      float centroids_dist2 = algebra::get_squared_distance(
                                           recs[i]->get_centroid(),
                                           recs[j]->get_centroid());
      if (centroids_dist2 < max_dist2) {
        float d2 = get_squared_distance(recs[i]->get_record(),
                                        recs[j]->get_record());
        if (d2 < max_dist2) {
          boost::add_edge(nodes[i],nodes[j],d2,g);
        }
      }}}}

template<class TransT>
int RMSDClustering<TransT>::cluster_graph(Graph &g,
                 const std::vector<TransformationRecord*> &recs,
                 float max_dist) {
  if (boost::num_edges(g)==0) return 0;
  IMP_LOG_VERBOSE("Going to cluster a graph of:"
          <<boost::num_vertices(g)<<std::endl);
  float max_dist2=max_dist*max_dist;
  //get all of the edge weights
  boost::property_map<Graph, boost::edge_weight_t>::type
    weight = get(boost::edge_weight, g);
  std::vector<std::pair<RCEdge,float> > edge_weight;
  RCEdgeIt ei, ei_end;
  for(boost::tie(ei,ei_end) = boost::edges(g); ei != ei_end; ++ei){
    edge_weight.push_back(std::pair<RCEdge,float>(*ei,
                                                  boost::get(weight,*ei)));
  }
  int num_joins=0;
  //sort the edges by weight
  std::sort(edge_weight.begin(),edge_weight.end(),sort_by_weight());
  //sort the edges
  std::vector<bool> used;
  used.insert(used.end(),boost::num_vertices(g),false);
  for(unsigned int i=0;i<edge_weight.size();i++) {
    RCEdge e = edge_weight[i].first;
    int v1_ind=boost::source(e,g);
    int v2_ind=boost::target(e,g);
    IMP_LOG_VERBOSE("Working on edge "<<i<<"bewteen nodes"<<v1_ind<<
            " and "<<v2_ind<<std::endl);
    //check if any end of the edge is deleted
    if (!used[v1_ind] && !used[v2_ind] &&
        (edge_weight[i].second < max_dist2)){
        ++num_joins;
        used[v1_ind] = true;
        used[v2_ind] = true;

        TransformationRecord* rec1 = recs[v1_ind];
        TransformationRecord* rec2 = recs[v2_ind];
        if (!(rec1->get_valid() &&rec2->get_valid())) continue;
        if (rec1->get_score() > rec2->get_score()) {
          rec1->join_into(*rec2);
          rec2->set_valid(false);
        } else {
          rec2->join_into(*rec1);
          rec1->set_valid(false);
        }
    }
  } // edges
  return num_joins;
}
template<class TransT>
void RMSDClustering<TransT>::prepare(const ParticlesTemp& ps) {
  rmsd_calc_=atom::RMSDCalculator(ps);
  // save centroid
  centroid_ = algebra::Vector3D(0,0,0);
  core::XYZs xyzs(ps);
  for (core::XYZs::iterator it =  xyzs.begin(); it != xyzs.end(); it++) {
    centroid_ += it->get_coordinates();
  }
  centroid_ /= ps.size();
  is_ready_=true;
}

template<class TransT>
int RMSDClustering<TransT>::fast_clustering(float max_dist,
       std::vector<TransformationRecord*>& recs) {
  IMP_LOG_VERBOSE("start fast clustering with "<<recs.size()<<" records\n");
  int num_joins = 0;
  boost::scoped_array<bool> used(new bool[recs.size()]);
  Hash3 g_hash((double)(bin_size_));

  //load the hash
  for (int i = 0 ; i < (int)recs.size() ; ++i){
    used[i] = false;
    TransT tr=recs[i]->get_record();
    algebra::Transformation3D t =
      tr.get_representative_transformation();
    algebra::Vector3D trans_cen = t.get_transformed(centroid_);
    g_hash.add(trans_cen, i);
    IMP_LOG_VERBOSE("add to hash vertex number:"<<i
            <<" with center:"<<trans_cen<<std::endl);
  }
  //work on each bucket
  const Hash3::GeomMap &M = g_hash.Map();
  for (Hash3::GeomMap::const_iterator bucket = M.begin();
       bucket != M.end() ; ++bucket){
    const Hash3::PointList &pb = bucket->second;
    IMP_LOG_VERBOSE("Bucket size:"<<pb.size()<<"\n");
    //    if (pb.size()<2) continue;
    Graph g;
    std::vector<std::pair<RCEdge,float> > edge_weight;
    build_graph(pb,recs,max_dist,g);
    IMP_LOG_VERBOSE("create graph with:"<<boost::num_vertices(g)<<" nodes and"<<
            boost::num_edges(g)<<" edges out of "<<pb.size()<<" points\n");
    //cluster all transformations in the bin
    num_joins +=cluster_graph(g,recs,max_dist);
    IMP_LOG_VERBOSE("after clustering number of joins::"<<num_joins<<std::endl);
  }
  return num_joins;
}


template<class TransT>
int RMSDClustering<TransT>::exhaustive_clustering(float max_dist,
           std::vector<TransformationRecord *>& recs) {
  IMP_LOG_VERBOSE("start full clustering with "<< recs.size()<<" records \n");
  if (recs.size()<2) return 0;
  boost::scoped_array<bool> used(new bool[recs.size()]);
  Hash3 ghash((double)(max_dist));

  //load the hash
  for (int i = 0 ; i < (int)recs.size() ; ++i) {
    used[i] = false;
    algebra::Transformation3D t =
                  recs[i]->get_record().get_representative_transformation();
    ghash.add(t.get_transformed(centroid_), i);
  }
  //build the graph
  Graph g;
  build_full_graph(ghash,recs,max_dist,g);
  int num_joins = cluster_graph(g,recs,max_dist);
  return num_joins;
}
template<class TransT>
void RMSDClustering<TransT>::clean(
                          std::vector<TransformationRecord*>*& records) {
  std::vector<TransformationRecord*> *results =
    new std::vector<TransformationRecord*>();
  for (int i = 0 ; i < (int)records->size() ; i++){
    if ((*records)[i]->get_valid()) {
      results->push_back((*records)[i]);
    } else {
      delete((*records)[i]);
    }
  }
  records->clear();
  delete records;
  records = results;
}
template<class TransT>
void RMSDClustering<TransT>::cluster(float max_dist,
        const std::vector<TransT> &input_trans,
        std::vector<TransT> & output) {
  //create initial vectors of transformation records and bit vector to
  //indicate what is deleted
  std::vector<TransformationRecord*>* records =
    new std::vector<TransformationRecord*>();
  for (typename std::vector<TransT>::const_iterator
         it = input_trans.begin();it != input_trans.end() ; ++it){
    TransformationRecord* record = new TransformationRecord(*it);
    record->set_centroid(centroid_);
    records->push_back(record);
  }
  //fast clustering using geometric hashing
  while (fast_clustering(max_dist, *records)){
    clean(records);
  }
  clean(records);
  //complete full clustering
  while (exhaustive_clustering(max_dist, *records)){
    clean(records);
  }
  //  clean(records);
  //build the vector for output
  IMP_LOG_VERBOSE("build output of "<<records->size()<<" records \n");
  for (int i = 0 ; i < (int)records->size() ; ++i){
    output.push_back((*records)[i]->get_record());
    delete((*records)[i]);
  }
  delete(records);
  IMP_LOG_VERBOSE("returning "<< output.size()<<" records \n");
}

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_RMSD_CLUSTERING_H */
