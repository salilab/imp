/**
 * \file IMP/kinematics/internal/graph_helpers.h
 * \brief Helper functions to deal with graphs.
 *
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_INTERNAL_GRAPH_HELPERS_H
#define IMPKINEMATICS_INTERNAL_GRAPH_HELPERS_H

#include "../kinematics_config.h"
#include <boost/graph/undirected_dfs.hpp>
#include <vector>
#include <iostream>

IMPKINEMATICS_BEGIN_INTERNAL_NAMESPACE

class MyDFSVisitor : public boost::default_dfs_visitor {
public:
  MyDFSVisitor(std::vector<int>& dfs_order, std::vector<int>& parents) :
    dfs_order_(dfs_order), parents_(parents), counter_(0) {}

  template < typename Vertex, typename Graph >
  void discover_vertex(Vertex v, const Graph& ) {
    dfs_order_[v] = counter_;
    counter_++;
  }

  template < typename Edge, typename Graph >
  void tree_edge(Edge e, const Graph& g) {
    std::cerr << "tree_edge " << source(e, g) << " -- " << target(e, g)
              << " dfs_order " <<  dfs_order_[source(e, g)] << " -- " <<  dfs_order_[target(e, g)] << std::endl;
    parents_[target(e,g)] = source(e, g);
  }

  std::vector<int>& dfs_order_;
  std::vector<int>& parents_;
  int counter_;
};

IMPKINEMATICS_END_INTERNAL_NAMESPACE

#endif /* IMPKINEMATICS_INTERNAL_GRAPH_HELPERS_H */
