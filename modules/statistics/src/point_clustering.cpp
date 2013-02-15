/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/point_clustering.h>
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/KMTerminationCondition.h>
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/base/vector_property_map.h>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/scoped_ptr.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE



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
  IMP_UNUSED(iterations);
  metric->set_was_used(true);
  IMP_USAGE_CHECK(k < iterations,
                  "You probably switched the k and iterations parameters."
                  << " The former should be (much) smaller than the latter.");
  if (names.size() < k) {
    IMP_THROW("Too few points to make into " << k << " clusters.",
              base::ValueException);
  }
  IMP_LOG_VERBOSE("KMLProxy::run start \n");
  //use the initial centers if provided
  boost::scoped_ptr<internal::KMPointArray> kmc;
  IMP_LOG_VERBOSE("KMLProxy::run load initial guess \n");
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
  IMP_LOG_VERBOSE("KMLProxy::run load lloyd \n");
  internal::KMTerminationCondition term;
  internal::KMLocalSearchLloyd la(&ctrs,&term);
  IMP_LOG_VERBOSE("KMLProxy::run excute lloyd \n");
  la.execute();
  internal::KMFilterCentersResults best_clusters = la.get_best();
  IMP_INTERNAL_CHECK(k
                     == (unsigned int) best_clusters.get_number_of_centers(),
             "The final number of centers does not match the requested one");
  IMP_LOG_VERBOSE("KMLProxy::run load best results \n");
  IMP::base::Vector<algebra::VectorKD> centers(k);
  for (unsigned int i = 0; i < k; i++) {
    internal::KMPoint *kmp = best_clusters[i];
    centers[i]=algebra::VectorKD(kmp->begin(), kmp->end());
  }
  //set the assignment of particles to centers
  //array of number of all points
  //TODO - return this
  IMP_LOG_VERBOSE("KMLProxy::run get assignments \n");
  const Ints &close_center = *best_clusters.get_assignments();
  IMP_LOG_VERBOSE("KMLProxy::run get assignments 2\n");
  IMP::base::Vector<Ints> clusters(k);
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
    IMP_THROW("No points to cluster", base::ValueException);
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
  IMP::base::Vector<Ints> clusters;
  IMP::base::Vector<algebra::VectorKD> centers;
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
  Grid grid(side, algebra::get_zero_vector_kd(dim));
  for (unsigned int i=0; i< embed->get_number_of_items(); ++i) {
    Grid::ExtendedIndex ei= grid.get_extended_index(embed->get_point(i));
    if (!grid.get_has_index(ei)) {
      grid.add_voxel(ei, Ints(1, i));
    } else {
      grid[grid.get_index(ei)].push_back(i);
    }
  }
  IMP::base::Vector<Ints> clusters;
  IMP::base::Vector<algebra::VectorKD> centers;
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




IMPSTATISTICS_END_NAMESPACE
