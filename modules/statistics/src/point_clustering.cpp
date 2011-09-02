/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/point_clustering.h>
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/KMTerminationCondition.h>
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/compatibility/vector_property_map.h>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/scoped_ptr.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE

namespace {
  algebra::Vector3D get_coordinates(Particle *p) {
    algebra::Vector3D ret(p->get_value(IMP::internal::xyzr_keys[0]),
                          p->get_value(IMP::internal::xyzr_keys[1]),
                          p->get_value(IMP::internal::xyzr_keys[2]));
    return ret;
  }
}

Embedding::Embedding(std::string name): Object(name){}

Embedding::~Embedding(){}


algebra::VectorKD VectorDEmbedding::get_point(unsigned int i) const {
  return vectors_[i];
}

unsigned int VectorDEmbedding::get_number_of_items() const {
  return vectors_.size();
}

void VectorDEmbedding::do_show(std::ostream &) const {
}

ConfigurationSetXYZEmbedding
::ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainer *sc,
                               bool align):
  Embedding("ConfiguringEmbedding"),
  cs_(cs), sc_(sc), align_(align){
}

algebra::VectorKD
ConfigurationSetXYZEmbedding::get_point(unsigned int a) const {
  algebra::Transformation3D tr= algebra::get_identity_transformation_3d();
  if (align_) {
    cs_->load_configuration(0);
    algebra::Vector3Ds vs0;
    IMP_FOREACH_SINGLETON(sc_, {
        vs0.push_back(get_coordinates(_1));
      });
    cs_->load_configuration(a);
    algebra::Vector3Ds vsc;
    IMP_FOREACH_SINGLETON(sc_, {
        vsc.push_back(get_coordinates(_1));
      });
    tr= get_transformation_aligning_first_to_second(vsc, vs0);
  } else {
    cs_->load_configuration(a);
  }
  Floats ret;
  IMP_FOREACH_SINGLETON(sc_, {
    algebra::Vector3D v
      = tr.get_transformed(get_coordinates(_1));
    ret.push_back(v[0]);
    ret.push_back(v[1]);
    ret.push_back(v[2]);
    });
  return algebra::VectorKD(ret.begin(), ret.end());
}

unsigned int ConfigurationSetXYZEmbedding::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetXYZEmbedding::do_show(std::ostream &out) const {
  cs_->show(out);
  sc_->show(out);
}


ParticleEmbedding::ParticleEmbedding(const ParticlesTemp &ps,
                                     const FloatKeys &ks,
                                     bool rescale):
  Embedding("ParticleEmbedding"),
  ps_(ps.begin(), ps.end()), ks_(ks), rescale_(rescale){
  if (rescale && !ps.empty()) {
    ranges_.resize(ks.size());
    for (unsigned int i=0; i< ks.size(); ++i) {
      FloatRange r=ps[0]->get_model()->get_range(ks[i]);
      ranges_[i]= FloatRange(r.first, 1.0/(r.second-r.first));
    }
  }
}

algebra::VectorKD ParticleEmbedding::get_point(unsigned int i) const {
  Floats ret(ks_.size());
  for (unsigned int j=0; j< ks_.size(); ++j) {
    ret[j]= ps_[i]->get_value(ks_[j]);
  }
  if (rescale_) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      ret[j]= (ret[j]-ranges_[j].first)*ranges_[j].second;
    }
  }
  return algebra::VectorKD(ret.begin(), ret.end());
}

unsigned int ParticleEmbedding::get_number_of_items() const {
  return ps_.size();
}

void ParticleEmbedding::do_show(std::ostream &out) const {
  for (unsigned int i=0; i< ks_.size(); ++i) {
    out << ks_[i] << " ";
  }
  out << std::endl;
}




unsigned int PartitionalClusteringWithCenter::get_number_of_clusters() const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
const Ints&PartitionalClusteringWithCenter::get_cluster(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  return clusters_[i];
}
int PartitionalClusteringWithCenter
::get_cluster_representative(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  return reps_[i];
}
void PartitionalClusteringWithCenter::do_show(std::ostream &out) const {
  out << centers_.size() << " centers." << std::endl;
}



namespace {

  algebra::VectorKDs get_all(Embedding *e) {
    algebra::VectorKDs ret(e->get_number_of_items());
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]= e->get_point(i);
    }
    return ret;
  }

PartitionalClusteringWithCenter*
create_lloyds_kmeans(const Ints &names, Embedding *metric,
                  unsigned int k, unsigned int iterations) {
  metric->set_was_used(true);
  IMP_USAGE_CHECK(k < iterations,
                  "You probably switched the k and iterations parameters."
                  << " The former should be (much) smaller than the latter.");
  if (names.size() < k) {
    IMP_THROW("Too few points to make into " << k << " clusters.",
              ValueException);
  }
  IMP_LOG(VERBOSE,"KMLProxy::run start \n");
  //use the initial centers if provided
  boost::scoped_ptr<internal::KMPointArray> kmc;
  IMP_LOG(VERBOSE,"KMLProxy::run load initial guess \n");
  //load the initail guess
  internal::KMData data(metric->get_point(names[0]).get_dimension(),
                        names.size());
  for (unsigned int i=0; i< names.size(); ++i) {
    algebra::VectorKD v= metric->get_point(names[i]);
    *(data[i])= Floats(v.coordinates_begin(),
                       v.coordinates_end());
  }
  internal::KMFilterCenters ctrs(k, &data, nullptr, 1.0);

  //apply lloyd search
  IMP_LOG(VERBOSE,"KMLProxy::run load lloyd \n");
  internal::KMTerminationCondition term;
  internal::KMLocalSearchLloyd la(&ctrs,&term);
  IMP_LOG(VERBOSE,"KMLProxy::run excute lloyd \n");
  la.execute();
  internal::KMFilterCentersResults best_clusters = la.get_best();
  IMP_INTERNAL_CHECK(k
                     == (unsigned int) best_clusters.get_number_of_centers(),
             "The final number of centers does not match the requested one");
  IMP_LOG(VERBOSE,"KMLProxy::run load best results \n");
  IMP::compatibility::checked_vector<algebra::VectorKD> centers(k);
  for (unsigned int i = 0; i < k; i++) {
    internal::KMPoint *kmp = best_clusters[i];
    centers[i]=algebra::VectorKD(kmp->begin(), kmp->end());
  }
  //set the assignment of particles to centers
  //array of number of all points
  //TODO - return this
  IMP_LOG(VERBOSE,"KMLProxy::run get assignments \n");
  const Ints &close_center = *best_clusters.get_assignments();
  IMP_LOG(VERBOSE,"KMLProxy::run get assignments 2\n");
  IMP::compatibility::checked_vector<Ints> clusters(k);
  for (unsigned int i=0;i<names.size();i++) {
    //std::cout<<"ps number i: " << i << " close center : "
    //<< (*close_center)[i] << std::endl;
    clusters[close_center[i]].push_back(names[i]);
  }
  Ints reps(k);
  for (unsigned int i=0; i< k; ++i) {
    int c=-1;
    double d= std::numeric_limits<double>::max();
    for (unsigned int j=0; j< clusters[i].size(); ++j) {
      Floats dc=*data[clusters[i][j]];
      double cd
        = algebra::get_distance(algebra::VectorKD(dc.begin(),
                                                  dc.end()),
                                centers[i]);
      if (cd < d) {
        d= cd;
        c= j;
      }
    }
    reps[i]=names[c];
  }

  IMP_NEW(PartitionalClusteringWithCenter, ret, (clusters, centers, reps));
  validate_partitional_clustering(ret, metric->get_number_of_items());
  return ret.release();
}
}

PartitionalClusteringWithCenter* create_lloyds_kmeans(Embedding *metric,
                                    unsigned int k, unsigned int iterations) {
  Ints names(metric->get_number_of_items());
  for (unsigned int i=0; i< names.size(); ++i) {
    names[i]=i;
  }
  if (names.empty()) {
    IMP_THROW("No points to cluster", ValueException);
  }
  return create_lloyds_kmeans(names, metric, k, iterations);
}


PartitionalClusteringWithCenter*
create_connectivity_clustering(Embedding *embed,
                            double dist) {
  IMP_USAGE_CHECK(embed->get_number_of_items() >0,
                  "There most be a point to clustering");
  IMP_NEW(algebra::NearestNeighborKD, nn,(get_all(embed), .1));
  typedef boost::vector_property_map<unsigned int> Index;
  typedef Index Parent;
  typedef boost::disjoint_sets<Index,Parent> UF;
  Index id;
  Parent pt;
  UF uf(id, pt);
  algebra::VectorKDs vs= get_all(embed);
  for (unsigned int i=0; i< vs.size(); ++i) {
    uf.make_set(i);
  }
  for (unsigned int i=0; i< vs.size(); ++i) {
    Ints ns= nn->get_in_ball(i, dist);
    for (unsigned int j=0; j < ns.size(); ++j) {
      if (get_distance(vs[i], vs[ns[j]]) < dist) {
        //std::cout << "Unioning " << i << " and " << ns[j] << std::endl;
        uf.union_set(static_cast<int>(i), ns[j]);
      }
    }
  }
  std::map<int,int> cluster_map;
  Ints reps;
  IMP::compatibility::checked_vector<Ints> clusters;
  IMP::compatibility::checked_vector<algebra::VectorKD> centers;
  for (unsigned int i=0; i < vs.size(); ++i) {
    int p= uf.find_set(i);
    if (cluster_map.find(p) == cluster_map.end()) {
      cluster_map[p]= clusters.size();
      clusters.push_back(Ints());
      centers.push_back(algebra::get_zero_vector_d<3>());
      reps.push_back(i);
    }
    int ci= cluster_map.find(p)->second;
    clusters[ci].push_back(i);
    centers[ci] += vs[i];
  }
  for (unsigned int i=0; i< clusters.size(); ++i) {
    centers[i]/= clusters[i].size();
    double md=std::numeric_limits<double>::max();
    for (unsigned int j=0; j < clusters[i].size(); ++j) {
      double d= get_distance(centers[i], vs[clusters[i][j]]);
      if (d < md) {
        md=d;
        reps[i]=clusters[i][j];
      }
    }
  }
  IMP_NEW(PartitionalClusteringWithCenter, ret, (clusters, centers, reps));
  validate_partitional_clustering(ret, embed->get_number_of_items());
  return ret.release();
}


PartitionalClusteringWithCenter*
create_bin_based_clustering(Embedding *embed,
                         double side) {
  IMP::OwnerPointer<Embedding> e(embed);
  typedef algebra::SparseUnboundedGridD<-1, Ints> Grid;
  int dim= embed->get_point(0).get_dimension();
  Grid grid(side, dim);
  for (unsigned int i=0; i< embed->get_number_of_items(); ++i) {
    Grid::ExtendedIndex ei= grid.get_extended_index(embed->get_point(i));
    if (!grid.get_has_index(ei)) {
      grid.add_voxel(ei, Ints(1, i));
    } else {
      grid[grid.get_index(ei)].push_back(i);
    }
  }
  IMP::compatibility::checked_vector<Ints> clusters;
  IMP::compatibility::checked_vector<algebra::VectorKD> centers;
  Ints reps;
  for (Grid::AllConstIterator it= grid.all_begin();
       it != grid.all_end(); ++it) {
    clusters.push_back(it->second);
    centers.push_back(grid.get_center(it->first));
    reps.push_back(clusters.back()[0]);
  }
  IMP_NEW(PartitionalClusteringWithCenter, ret, (clusters, centers, reps));
  validate_partitional_clustering(ret, embed->get_number_of_items());
  return ret.release();

}


PartitionalClustering *create_centrality_clustering(Embedding *d,
                                                 double far,
                                                 int k) {
  IMP::OwnerPointer<Embedding> dp(d);
  const unsigned int n=d->get_number_of_items();
  algebra::VectorKDs vs= get_all(d);
  IMP_NEW(algebra::NearestNeighborKD, nn, (vs));
  internal::CentralityGraph g(n);
  boost::property_map<internal::CentralityGraph,
                      boost::edge_weight_t>::type w
    = boost::get(boost::edge_weight, g);

  for (unsigned int i=0; i<n; ++i) {
    Ints cnn= nn->get_in_ball(i, far);
    for (unsigned int j=0; j< cnn.size(); ++j) {
      double dist= algebra::get_distance(vs[i], vs[j]);
      boost::graph_traits<internal::CentralityGraph>::edge_descriptor e
          =add_edge(i,j, g).first;
        w[e]=/*1.0/*/dist;
    }
  }
  return internal::get_centrality_clustering(g, k);
}



algebra::VectorKDs get_centroids(Embedding* d, PartitionalClustering *pc) {
  Pointer<Embedding> pd(d);
  Pointer<PartitionalClustering> ppc(pc);
  algebra::VectorKDs ret(pc->get_number_of_clusters());
  for (unsigned int i=0; i< ret.size(); ++i) {
    algebra::VectorKD cur
      = algebra::get_zero_vector_kd(d->get_point(0).get_dimension());
    Ints cc= pc->get_cluster(i);
    for (unsigned int j=0; j< cc.size(); ++j) {
      cur+= d->get_point(cc[j]);
    }
    cur/= cc.size();
    ret[i]=cur;
  }
  return ret;
}

Ints get_representatives(Embedding* d, PartitionalClustering *pc) {
  Pointer<Embedding> pd(d);
  Pointer<PartitionalClustering> ppc(pc);
  algebra::VectorKDs centroids= get_centroids(d, pc);
  Ints ret(centroids.size());
  IMP_NEW(algebra::NearestNeighborKD, nn, (get_all(d)));
  for (unsigned int i=0; i < centroids.size(); ++i) {
    ret[i]= nn->get_nearest_neighbors(centroids[i], 1)[0];
  }
  return ret;
}





RecursivePartitionalClusteringEmbedding
::RecursivePartitionalClusteringEmbedding(Embedding *metric,
                                       PartitionalClustering *clustering):
  Embedding("RecursivePartitionalClusteringEmbedding %1%"),
  metric_(metric), clustering_(clustering){

}

PartitionalClustering*
 RecursivePartitionalClusteringEmbedding
::create_full_clustering(PartitionalClustering *center_cluster) {
  IMP::compatibility::checked_vector<Ints>
    clusters(center_cluster->get_number_of_clusters());
  Ints reps(clusters.size());
  for (unsigned int i=0; i< clusters.size(); ++i) {
    Ints outer= center_cluster->get_cluster(i);
    reps[i]=clustering_->get_cluster_representative(
               center_cluster->get_cluster_representative(i));
    for (unsigned int j=0; j< outer.size(); ++j) {
      Ints inner= clustering_->get_cluster(outer[j]);
      clusters[i].insert(clusters[i].end(),inner.begin(), inner.end());
    }
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters, reps));
  validate_partitional_clustering(ret, metric_->get_number_of_items());
  return ret.release();
}


algebra::VectorKD
RecursivePartitionalClusteringEmbedding::get_point(unsigned int i) const {
  return metric_->get_point(clustering_->get_cluster_representative(i));
}

unsigned int
RecursivePartitionalClusteringEmbedding::get_number_of_items() const {
  return clustering_->get_number_of_clusters();
}

void RecursivePartitionalClusteringEmbedding::do_show(std::ostream &) const {
}



IMPSTATISTICS_END_NAMESPACE
