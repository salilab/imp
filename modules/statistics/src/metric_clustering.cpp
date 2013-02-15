/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/metric_clustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/atom/distance.h>
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/base/vector_property_map.h>
#include <boost/pending/disjoint_sets.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE


PartitionalClustering *create_centrality_clustering(Metric *d,
                                                 double far,
                                                 int k) {
  IMP::OwnerPointer<Metric> dp(d);
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
          IMP::base::Vector<Floats>& matrix) {
    IMP_LOG_TERSE( "Extracting distance matrix..." << std::endl);
    matrix
      =IMP::base::Vector<Floats>(d->get_number_of_items(),
                                    Floats(d->get_number_of_items(), 0));
    for (unsigned int i=0; i< matrix.size(); ++i) {
      for (unsigned int j=0; j< i; ++j) {
        matrix[i][j]= d->get_distance(i,j);
        matrix[j][i]= matrix[i][j];
      }
      matrix[i][i]=0;
    }
    IMP_LOG_TERSE( "done" << std::endl);
  }
  double get_min_distance(int cur,
            const IMP::base::Vector<Ints> &clusters,
               const IMP::base::Vector<Floats>& matrix) {
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
              const IMP::base::Vector<Ints> &clusters,
              const IMP::base::Vector<Floats>& matrix) {
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
  IMP::OwnerPointer<Metric> mp(d);
  IMP_FUNCTION_LOG;
  IMP::base::Vector<Floats> matrix;
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
  IMP::base::Vector<Ints> clusters;
  for (unsigned int i=0; i < matrix.size(); ++i) {
    int p= uf.find_set(i);
    if (cluster_map.find(p) == cluster_map.end()) {
      cluster_map[p]= clusters.size();
      clusters.push_back(Ints());
    }
    int ci= cluster_map.find(p)->second;
    clusters[ci].push_back(i);
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}


PartitionalClustering *create_diameter_clustering(Metric *d,
                                                  double maximum_diameter) {
  IMP::OwnerPointer<Metric> mp(d);
  IMP_FUNCTION_LOG;
  IMP::base::Vector<Floats> matrix;
  fill_distance_matrix(d, matrix);
  IMP::base::Vector<Ints> clusters;
  Ints unclaimed(matrix.size());
  for (unsigned int i=0; i< matrix.size(); ++i) {
    unclaimed[i]=i;
  }
  while (!unclaimed.empty()) {
    clusters.push_back(Ints());
    int cur= get_far(unclaimed, clusters, matrix);
    clusters.back().push_back(unclaimed[cur]);
    IMP_LOG_VERBOSE( "Adding cluster around " << unclaimed[cur] << std::endl);
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
        IMP_LOG_VERBOSE( "Adding " << unclaimed[i]
                << " to cluster." << std::endl);
        unclaimed.erase(unclaimed.begin()+i);
      }
    }
  }
  base::Vector<Ints> goodclusters;
  std::copy(clusters.begin(), clusters.end(),
            std::back_inserter(goodclusters));
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}


IMPSTATISTICS_END_NAMESPACE
