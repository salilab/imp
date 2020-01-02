/**
 *  \file KMRectangle.cpp   \brief Orthogonal (axis aligned) rectangle
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/centrality_clustering.h>
#include <IMP/statistics/PartitionalClustering.h>
#include <IMP/statistics/internal/TrivialPartitionalClustering.h>

// Work around Boost bug with adjacency_matrix in 1.60:
// https://svn.boost.org/trac/boost/ticket/11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
# include <boost/type_traits/ice.hpp>
#endif

#include <boost/property_map/vector_property_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/unordered_map.hpp>
#if BOOST_VERSION > 103900
namespace boost {
// work around bug in bc_clustering
using graph::has_no_edges;
}
#endif

// Add workaround for https://github.com/boostorg/graph/issues/175
// (Note that this may not be the right #if condition; we have only seen
// the issue with Boost 1.71 or 1.72 on Macs with Homebrew, but other
// systems may be affected.)
#if defined(__clang__) && BOOST_VERSION >= 107100 && BOOST_VERSION <= 107200
# include "our_bc_clustering.hpp"
#else
# include <boost/graph/bc_clustering.hpp>
#endif

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

namespace {
/*struct centrality_t {
  typedef boost::edge_property_tag kind;
  } centrality;*/
typedef boost::adjacency_matrix<
    boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, double,
                    boost::property<boost::edge_centrality_t, double> > > Graph;
/*typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::no_property,
                              boost::property<boost::edge_weight_t,
                              double> > Graph;*/
typedef boost::graph_traits<Graph> Traits;

typedef boost::vector_property_map<Traits::vertex_descriptor> PM;

typedef boost::disjoint_sets<PM, PM> DS;

struct Done {
  typedef double centrality_type;
  int k_;
  PM rank_, parent_;
  Done(int k, int n) : k_(k), rank_(n), parent_(n) {}
  template <class B>
  bool operator()(centrality_type c, const B &e, const Graph &g) {
    /*      std::cout << "Done called on " << boost::source(e, g)
                    << "--" << boost::target(e, g)
                    << ": " << c << std::endl;
    */
    IMP_UNUSED(c);
    IMP_UNUSED(e);
    DS ds(rank_, parent_);
    boost::initialize_incremental_components(g, ds);
    boost::incremental_components(g, ds);
    unsigned int s =
        ds.count_sets(boost::vertices(g).first, boost::vertices(g).second);
    return s >= static_cast<unsigned int>(k_);
  }
};
}

PartitionalClustering *get_centrality_clustering(CentralityGraph &g,
                                                 unsigned int k) {
  boost::property_map<CentralityGraph, boost::edge_centrality_t>::type m =
      boost::get(boost::edge_centrality, g);
  unsigned int n = boost::num_vertices(g);
  boost::betweenness_centrality_clustering(g, Done(k, n), m);
  PM rank(n), parent(n);
  DS ds(rank, parent);
  boost::initialize_incremental_components(g, ds);
  boost::incremental_components(g, ds);
  boost::unordered_map<int, Ints> sets;
  for (unsigned int i = 0; i < boost::num_vertices(g); ++i) {
    int s = ds.find_set(i);
    sets[s].push_back(i);
  }
  Vector<Ints> clusters;
  for (boost::unordered_map<int, Ints>::const_iterator it = sets.begin();
       it != sets.end(); ++it) {
    clusters.push_back(it->second);
  }
  IMP_NEW(internal::TrivialPartitionalClustering, ret, (clusters));
  return ret.release();
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE
