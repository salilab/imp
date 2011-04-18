/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/metric_clustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/atom/distance.h>
#include <IMP/statistics/internal/centrality_clustering.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif
#include <boost/pending/disjoint_sets.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE

Metric::Metric(std::string name): Object(name){}

Metric::~Metric(){}

EuclideanMetric::EuclideanMetric(Embedding *em):
  Metric("Euclidean %1%"), em_(em){}
double EuclideanMetric::get_distance(unsigned int i,
                                       unsigned int j) const {
  return algebra::get_distance(em_->get_point(i), em_->get_point(j));
}

unsigned int EuclideanMetric::get_number_of_items() const {
  return em_->get_number_of_items();
}

void EuclideanMetric::do_show(std::ostream &) const {
}


ConfigurationSetRMSDMetric::ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                                                         SingletonContainer *sc,
                                                         bool align):
  Metric("CS RMS %1%"),
  cs_(cs), sc_(sc), align_(align){

}

namespace {
  algebra::Vector3Ds get_vectors(ConfigurationSet *cs,
                        unsigned int i,
                        SingletonContainer *sc) {
    algebra::Vector3Ds ret(sc->get_number_of_particles());
    cs->load_configuration(i);
    IMP_FOREACH_SINGLETON(sc, {
        ret[_2]= _1->_get_coordinates().get_center();
      });
    return ret;
  }
}

double ConfigurationSetRMSDMetric::get_distance(unsigned int i,
                                                 unsigned int j) const {
  algebra::Vector3Ds vi= get_vectors(cs_, i, sc_);
  algebra::Vector3Ds vj= get_vectors(cs_, j, sc_);
  algebra::Transformation3D tr;
  if (align_) {
    tr= algebra::get_transformation_aligning_first_to_second(vi, vj);
  } else {
    tr=algebra::get_identity_transformation_3d();
  }
  return atom::get_rmsd(vi, vj, tr);
}

unsigned int ConfigurationSetRMSDMetric::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetRMSDMetric::do_show(std::ostream &) const {
}


PartitionalClustering *create_centrality_clustering(Metric *d,
                                                 double far,
                                                 int k) {
  IMP::internal::OwnerPointer<Metric> dp(d);
  unsigned int n=d->get_number_of_items();
  internal::CentralityGraph g(n);
  boost::property_map<internal::CentralityGraph,
                      boost::edge_weight_t>::type w
    = boost::get(boost::edge_weight, g);

  for (unsigned int i=0; i<n; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      double dist= d->get_distance(i,j);
      if (dist < far) {
        boost::graph_traits<internal::CentralityGraph>::edge_descriptor e
          =add_edge(i,j, g).first;
        w[e]=/*1.0/*/dist;
      }
    }
  }
  return internal::get_centrality_clustering(g, k);
}

namespace {
  void fill_distance_matrix(Metric *d,
                            std::vector<Floats>& matrix) {
    IMP_LOG(TERSE, "Extracting distance matrix..." << std::endl);
    matrix=std::vector<Floats>(d->get_number_of_items(),
                               Floats(d->get_number_of_items(), 0));
    for (unsigned int i=0; i< matrix.size(); ++i) {
      for (unsigned int j=0; j< i; ++j) {
        matrix[i][j]= d->get_distance(i,j);
        matrix[j][i]= matrix[i][j];
      }
      matrix[i][i]=0;
    }
    IMP_LOG(TERSE, "done" << std::endl);
  }
  double get_min_distance(int cur, const std::vector<Ints> &clusters,
                          const std::vector<Floats>& matrix) {
    double ret=std::numeric_limits<double>::max();
    for (unsigned int i=0; i< clusters.size(); ++i) {
      for (unsigned int j=0; j< clusters[i].size(); ++j) {
        double d= matrix[cur][clusters[i][j]];
        ret=std::min(d, ret);
      }
    }
    return ret;
  }
  int get_far(const Ints &unclaimed,
              const std::vector<Ints> &clusters,
              const std::vector<Floats>& matrix) {
    if (clusters.empty()) return unclaimed.size()-1;
    double mdf=0;
    int mdi=-1;
    for (unsigned int i=0; i< unclaimed.size(); ++i) {
      double cd= get_min_distance(unclaimed[i], clusters, matrix);
      if (cd > mdf) {
        mdf=cd;
        mdi=i;
      }
    }
    return mdi;
  }
}

PartitionalClustering *create_connectivity_clustering(Metric *d,
                                                      double maximum_distance) {
  IMP::internal::OwnerPointer<Metric> mp(d);
  IMP_FUNCTION_LOG;
  std::vector<Floats> matrix;
  fill_distance_matrix(d, matrix);
  typedef boost::vector_property_map<unsigned int> Index;
  typedef Index Parent;
  typedef boost::disjoint_sets<Index,Parent> UF;
  Index id;
  Parent pt;
  UF uf(id, pt);
  for (unsigned int i=0; i< matrix.size(); ++i) {
    uf.make_set(i);
  }
  for (unsigned int i=0; i< matrix.size(); ++i) {
    for (unsigned int j=0; j < i; ++j) {
      if (matrix[i][j] < maximum_distance) {
        //std::cout << "Unioning " << i << " and " << ns[j] << std::endl;
        uf.union_set(static_cast<int>(i), static_cast<int>(j));
      }
    }
  }
  std::map<int,int> cluster_map;
  std::vector<Ints> clusters;
  for (unsigned int i=0; i < matrix.size(); ++i) {
    int p= uf.find_set(i);
    if (cluster_map.find(p) == cluster_map.end()) {
      cluster_map[p]= clusters.size();
      clusters.push_back(Ints());
    }
    int ci= cluster_map.find(p)->second;
    clusters[ci].push_back(i);
  }
  return new internal::TrivialPartitionalClustering(clusters);
}


PartitionalClustering *create_diameter_clustering(Metric *d,
                                                  double maximum_diameter) {
  IMP::internal::OwnerPointer<Metric> mp(d);
  IMP_FUNCTION_LOG;
  std::vector<Floats> matrix;
  fill_distance_matrix(d, matrix);
  std::vector<Ints> clusters;
  Ints unclaimed(matrix.size());
  for (unsigned int i=0; i< matrix.size(); ++i) {
    unclaimed[i]=i;
  }
  while (!unclaimed.empty()) {
    clusters.push_back(Ints());
    int cur= get_far(unclaimed, clusters, matrix);
    clusters.back().push_back(cur);
    unclaimed.erase(unclaimed.begin()+cur);
    for ( int i=unclaimed.size()-1; i>=0; --i) {
      bool bad=0;
      for (unsigned int j=0; j< clusters.back().size(); ++j) {
        if (matrix[clusters.back()[j]][unclaimed[i]] > maximum_diameter) {
          bad=true;
          break;
        }
      }
      if (!bad) {
        clusters.back().push_back(unclaimed[i]);
        unclaimed.erase(unclaimed.begin()+i);
      }
    }
  }
  return new internal::TrivialPartitionalClustering(clusters);
}



RecursivePartitionalClusteringMetric
::RecursivePartitionalClusteringMetric(Metric *metric,
                                       PartitionalClustering *clustering):
  Metric("RecursivePartitionalClusteringMetric %1%"),
  metric_(metric), clustering_(clustering){

}

PartitionalClustering*
 RecursivePartitionalClusteringMetric
::create_full_clustering(PartitionalClustering *center_cluster) {
  std::vector<Ints> clusters(center_cluster->get_number_of_clusters());
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
  return new internal::TrivialPartitionalClustering(clusters, reps);
}


double RecursivePartitionalClusteringMetric::get_distance(unsigned int i,
                                       unsigned int j) const {
  return metric_->get_distance(clustering_->get_cluster_representative(i),
                               clustering_->get_cluster_representative(j));
}

unsigned int RecursivePartitionalClusteringMetric::get_number_of_items() const {
  return clustering_->get_number_of_clusters();
}

void RecursivePartitionalClusteringMetric::do_show(std::ostream &) const {
}


IMPSTATISTICS_END_NAMESPACE
