/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/distance_clustering.h>
#include <IMP/core/XYZ.h>
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/KMTerminationCondition.h>
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/atom/distance.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/graph/graph_utility.hpp>
#if BOOST_VERSION > 103900
namespace boost {
  // work around bug in bc_clustering
 using graph::has_no_edges;
}
#endif

#include <boost/graph/bc_clustering.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE

Distance::Distance(std::string name): Object(name){}

Distance::~Distance(){}


double EuclideanDistance::get_distance(unsigned int i,
                                       unsigned int j) const {
  return algebra::get_distance(vectors_[i], vectors_[j]);
}

unsigned int EuclideanDistance::get_number_of_items() const {
  return vectors_.size();
}

void EuclideanDistance::do_show(std::ostream &) const {
}


ConfigurationSetRMSDistance::ConfigurationSetRMSDistance(ConfigurationSet *cs,
                                                         SingletonContainer *sc,
                                                         bool align):
  Distance("CS RMS %1%"),
  cs_(cs), sc_(sc), align_(align){

}

namespace {
  algebra::Vector3Ds get_vectors(ConfigurationSet *cs,
                        unsigned int i,
                        SingletonContainer *sc) {
    algebra::Vector3Ds ret(sc->get_number_of_particles());
    cs->load_configuration(i);
    IMP_FOREACH_SINGLETON(sc, {
        ret[_2]= core::XYZ(_1).get_coordinates();
      });
    return ret;
  }
}

double ConfigurationSetRMSDistance::get_distance(unsigned int i,
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

unsigned int ConfigurationSetRMSDistance::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetRMSDistance::do_show(std::ostream &) const {
}


namespace {
  /*struct centrality_t {
    typedef boost::edge_property_tag kind;
    } centrality;*/
  typedef boost::adjacency_matrix<boost::undirectedS,
                                  boost::no_property,
                                  boost::property<boost::edge_weight_t,
                                                  double,
            boost::property<boost::edge_centrality_t, double> > > Graph;
  /*typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::undirectedS,
                                boost::no_property,
                                boost::property<boost::edge_weight_t,
                                double> > Graph;*/
  typedef boost::graph_traits<Graph> Traits;

  typedef boost::disjoint_sets<int*, int*> DS;

  struct Done {
    typedef double centrality_type;
    int k_;
    std::vector<int> rank_, parent_;
    Done(int k, int n): k_(k), rank_(n), parent_(n){}
    template <class B>
    bool operator()(centrality_type c, const B & e, const Graph &g) {
      std::cout << "Done called on " << boost::source(e, g)
                << "--" << boost::target(e, g)
                << ": " << c << std::endl;
      DS ds(&rank_[0], &parent_[0]);
      boost::initialize_incremental_components(g, ds);
      boost::incremental_components(g, ds);
      unsigned int s= ds.count_sets(boost::vertices(g).first,
                                    boost::vertices(g).second);
      return s >= static_cast<unsigned int>(k_);
    }
  };


class IMPSTATISTICSEXPORT TrivialPartitionalClustering:
  public PartitionalClustering {
  std::vector<Ints> clusters_;
public:
  TrivialPartitionalClustering(const std::vector<Ints> &clusters):
    PartitionalClustering("trivial"),
    clusters_(clusters){}
  IMP_CLUSTERING(TrivialPartitionalClustering);
};


unsigned int TrivialPartitionalClustering::get_number_of_clusters() const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
const Ints&TrivialPartitionalClustering::get_cluster(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  set_was_used(true);
  return clusters_[i];
}
int TrivialPartitionalClustering
::get_cluster_representative(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  return clusters_[i][0];
}
void TrivialPartitionalClustering::do_show(std::ostream &out) const {
  out << clusters_.size() << " centers." << std::endl;
}

}

IMPSTATISTICSEXPORT
PartitionalClustering *get_centrality_clustering(Distance *d,
                                                 double far,
                                                 int k) {
  IMP::internal::OwnerPointer<Distance> dp(d);
  unsigned int n=d->get_number_of_items();
  Graph g(n);
  boost::property_map<Graph, boost::edge_weight_t>::type w
    = boost::get(boost::edge_weight, g);

  for (unsigned int i=0; i<n; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      double dist= d->get_distance(i,j);
      if (dist < far) {
        boost::graph_traits<Graph>::edge_descriptor e
          =add_edge(i,j, g).first;
        w[e]=/*1.0/*/dist;
      }
    }
  }
  boost::property_map<Graph, boost::edge_centrality_t>::type m
    = boost::get(boost::edge_centrality, g);
  boost::betweenness_centrality_clustering(g, Done(k, n), m);
  std::vector<int> rank(n), parent(n);
  DS ds(&rank[0], &parent[0]);
  boost::initialize_incremental_components(g, ds);
  boost::incremental_components(g, ds);
  IMP::internal::Map<int, Ints> sets;
  for (unsigned int i=0; i< n; ++i) {
    int s= ds.find_set(i);
    sets[s].push_back(i);
  }
  std::vector<Ints> clusters;
  for (IMP::internal::Map<int, Ints>::const_iterator it
         = sets.begin(); it != sets.end(); ++it) {
    clusters.push_back(it->second);
  }
  IMP_NEW(TrivialPartitionalClustering, ret, (clusters));
  return ret.release();
}

IMPSTATISTICS_END_NAMESPACE
