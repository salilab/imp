/**
 *  \file random_generator.h   \brief random number generator
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_CENTRALITY_CLUSTERING_H
#define IMPSTATISTICS_INTERNAL_CENTRALITY_CLUSTERING_H

#include <IMP/statistics/statistics_config.h>

// Work around Boost bug with adjacency_matrix in 1.60:
// https://svn.boost.org/trac/boost/ticket/11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
# include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>

IMPSTATISTICS_BEGIN_NAMESPACE
class PartitionalClustering;
IMPSTATISTICS_END_NAMESPACE
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
/*struct centrality_t {
  typedef boost::edge_property_tag kind;
  } centrality;*/
typedef boost::adjacency_matrix<
    boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, double,
                    boost::property<boost::edge_centrality_t, double> > >
    CentralityGraph;
/*typedef boost::adjacency_list<boost::vecS, boost::vecS,
  boost::undirectedS,
  boost::no_property,
  boost::property<boost::edge_weight_t,
  double> > Graph;*/
typedef boost::graph_traits<CentralityGraph> CentralityTraits;

IMPSTATISTICSEXPORT PartitionalClustering *get_centrality_clustering(
    CentralityGraph &g, unsigned int k);

IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_INTERNAL_CENTRALITY_CLUSTERING_H */
