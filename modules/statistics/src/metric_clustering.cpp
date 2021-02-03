/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/metric_clustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/vector_property_map.h>
#include <boost/pending/disjoint_sets.hpp>
#include <IMP/Pointer.h>

IMPSTATISTICS_BEGIN_NAMESPACE

PartitionalClustering *create_centrality_clustering(Metric *d, double far,
                                                    int k) {
  IMP::Pointer<Metric> dp(d);  // TODO: nothing is done with dp?
  unsigned int n = d->get_number_of_items();
  internal::CentralityGraph g(n);
  boost::property_map<internal::CentralityGraph, boost::edge_weight_t>::type w =
      boost::get(boost::edge_weight, g);

  for (unsigned int i = 0; i < n; ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      double dist = d->get_distance(i, j);
      if (dist < far) {
        boost::graph_traits<internal::CentralityGraph>::edge_descriptor e =
            add_edge(i, j, g).first;
        w[e] = /*1.0/*/ dist;
      }
    }
  }
  return internal::get_centrality_clustering(g, k);
}

namespace {
void fill_distance_matrix(Metric *d, IMP::Vector<Floats> &matrix) {
  IMP_LOG_TERSE("Extracting distance matrix..." << std::endl);
  matrix = IMP::Vector<Floats>(d->get_number_of_items(),
                                     Floats(d->get_number_of_items(), 0));
  for (unsigned int i = 0; i < matrix.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      matrix[i][j] = d->get_distance(i, j);
      matrix[j][i] = matrix[i][j];
    }
    matrix[i][i] = 0;
  }
  IMP_LOG_TERSE("done" << std::endl);
}
double get_min_distance(int cur, const IMP::Vector<Ints> &clusters,
                        const IMP::Vector<Floats> &matrix) {
  double ret = std::numeric_limits<double>::max();
  for (unsigned int i = 0; i < clusters.size(); ++i) {
    for (unsigned int j = 0; j < clusters[i].size(); ++j) {
      double d = matrix[cur][clusters[i][j]];
      ret = std::min(d, ret);
    }
  }
  return ret;
}
int get_far(const Ints &unclaimed, const IMP::Vector<Ints> &clusters,
            const IMP::Vector<Floats> &matrix) {
  if (clusters.empty()) return unclaimed.size() - 1;
  double mdf = 0;
  int mdi = -1;
  for (unsigned int i = 0; i < unclaimed.size(); ++i) {
    double cd = get_min_distance(unclaimed[i], clusters, matrix);
    if (cd > mdf) {
      mdf = cd;
      mdi = i;
    }
  }
  return mdi;
}
}

PartitionalClustering *create_connectivity_clustering(Metric *d,
                                                      double maximum_distance) {
  IMP::PointerMember<Metric> mp(d);
  IMP_FUNCTION_LOG;
  IMP::Vector<Floats> matrix;
  fill_distance_matrix(d, matrix);
  typedef boost::vector_property_map<unsigned int> Index;
  typedef Index Parent;
  typedef boost::disjoint_sets<Index, Parent> UF;
  Index id;
  Parent pt;
  UF uf(id, pt);
  for (unsigned int i = 0; i < matrix.size(); ++i) {
    uf.make_set(i);
  }
  for (unsigned int i = 0; i < matrix.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      if (matrix[i][j] < maximum_distance) {
        // std::cout << "Unioning " << i << " and " << ns[j] << std::endl;
        uf.union_set(static_cast<int>(i), static_cast<int>(j));
      }
    }
  }
  std::map<int, int> cluster_map;
  IMP::Vector<Ints> clusters;
  for (unsigned int i = 0; i < matrix.size(); ++i) {
    int p = uf.find_set(i);
    if (cluster_map.find(p) == cluster_map.end()) {
      cluster_map[p] = clusters.size();
      clusters.push_back(Ints());
    }
    int ci = cluster_map.find(p)->second;
    clusters[ci].push_back(i);
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}

PartitionalClustering *create_diameter_clustering(Metric *d,
                                                  double maximum_diameter) {
  IMP::PointerMember<Metric> mp(d);
  IMP_FUNCTION_LOG;
  IMP::Vector<Floats> matrix;
  fill_distance_matrix(d, matrix);
  IMP::Vector<Ints> clusters;
  Ints unclaimed(matrix.size());
  for (unsigned int i = 0; i < matrix.size(); ++i) {
    unclaimed[i] = i;
  }
  while (!unclaimed.empty()) {
    clusters.push_back(Ints());
    int cur = get_far(unclaimed, clusters, matrix);
    clusters.back().push_back(unclaimed[cur]);
    IMP_LOG_VERBOSE("Adding cluster around " << unclaimed[cur] << std::endl);
    unclaimed.erase(unclaimed.begin() + cur);
    for (int i = unclaimed.size() - 1; i >= 0; --i) {
      bool bad = 0;
      for (unsigned int j = 0; j < clusters.back().size(); ++j) {
        if (matrix[clusters.back()[j]][unclaimed[i]] > maximum_diameter) {
          bad = true;
          break;
        }
      }
      if (!bad) {
        clusters.back().push_back(unclaimed[i]);
        IMP_LOG_VERBOSE("Adding " << unclaimed[i] << " to cluster."
                                  << std::endl);
        unclaimed.erase(unclaimed.begin() + i);
      }
    }
  }
  Vector<Ints> goodclusters;
  std::copy(clusters.begin(), clusters.end(), std::back_inserter(goodclusters));
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}

PartitionalClustering *create_gromos_clustering(Metric *d, double cutoff) {
  IntsList clusters;
  unsigned nitems = d->get_number_of_items();

  // create vector of neighbors and weights
  std::vector<Ints> neighbors(nitems);
  Floats weights(nitems);
  for (unsigned i = 0; i < nitems; ++i) {
    neighbors[i].push_back(static_cast<int>(i));
    weights[i] = d->get_weight(i);
  }
  for (unsigned i = 0; i < nitems - 1; ++i) {
    for (unsigned j = i + 1; j < nitems; ++j) {
      if (d->get_distance(i, j) < cutoff) {
        neighbors[i].push_back(static_cast<int>(j));
        neighbors[j].push_back(static_cast<int>(i));
        weights[i] += d->get_weight(j);
        weights[j] += d->get_weight(i);
      }
    }
  }

  double maxweight = 1.0;
  while (maxweight > 0.0) {
    // find the conf with maximum weight
    maxweight = -1.0;
    int icenter = -1;
    for (unsigned i = 0; i < weights.size(); ++i) {
      if (weights[i] > maxweight) {
        maxweight = weights[i];
        icenter = i;
      }
    }
    IMP_INTERNAL_CHECK(icenter >= 0, "No center found");
    // no more clusters to find
    if (maxweight < 0.) {
      break;
    }

    // create the new cluster
    Ints newcluster = neighbors[icenter];
    clusters.push_back(newcluster);

    // remove from pool
    for (unsigned i = 0; i < newcluster.size(); ++i) {
      unsigned k = 0;
      while (k < neighbors.size()) {
        if (neighbors[k][0] == newcluster[i]) {
          // eliminate the entire neighbor list
          neighbors.erase(neighbors.begin() + k);
          weights.erase(weights.begin() + k);
        } else {
          // and the element in all the other neighbor lists
          Ints::iterator it =
              find(neighbors[k].begin(), neighbors[k].end(), newcluster[i]);
          if (it != neighbors[k].end()) {
            neighbors[k].erase(it);
            weights[k] -= d->get_weight(newcluster[i]);
          }
          k++;
        }
      }
    }
  }
  IMP_NEW(statistics::internal::TrivialPartitionalClustering, ret, (clusters));
  statistics::validate_partitional_clustering(ret, d->get_number_of_items());
  return ret.release();
}

IMPSTATISTICS_END_NAMESPACE
