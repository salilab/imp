/**
 *  \file KMRectangle.cpp   \brief Orthogonal (axis aligned) rectangle
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/statistics/PartitionalClustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>
#include <IMP/Pointer.h>
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

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE


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
    Ints rank_, parent_;
    Done(int k, int n): k_(k), rank_(n), parent_(n){}
    template <class B>
    bool operator()(centrality_type c, const B & e, const Graph &g) {
/*      std::cout << "Done called on " << boost::source(e, g)
                << "--" << boost::target(e, g)
                << ": " << c << std::endl;
*/
      IMP_UNUSED(c);
      IMP_UNUSED(e);
      DS ds(&rank_[0], &parent_[0]);
      boost::initialize_incremental_components(g, ds);
      boost::incremental_components(g, ds);
      unsigned int s= ds.count_sets(boost::vertices(g).first,
                                    boost::vertices(g).second);
      return s >= static_cast<unsigned int>(k_);
    }
  };

}

PartitionalClustering *get_centrality_clustering(CentralityGraph &g,
                                              unsigned int k) {
  boost::property_map<CentralityGraph, boost::edge_centrality_t>::type m
    = boost::get(boost::edge_centrality, g);
  unsigned int n=boost::num_vertices(g);
  boost::betweenness_centrality_clustering(g, Done(k, n),
                                           m);
  Ints rank(n), parent(n);
  DS ds(&rank[0], &parent[0]);
  boost::initialize_incremental_components(g, ds);
  boost::incremental_components(g, ds);
  IMP::base::map<int, Ints> sets;
  for (unsigned int i=0; i< boost::num_vertices(g); ++i) {
    int s= ds.find_set(i);
    sets[s].push_back(i);
  }
  IMP::base::Vector<Ints> clusters;
  for (IMP::base::map<int, Ints>::const_iterator it
         = sets.begin(); it != sets.end(); ++it) {
    clusters.push_back(it->second);
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  return ret.release();
}


IMPSTATISTICS_END_INTERNAL_NAMESPACE
