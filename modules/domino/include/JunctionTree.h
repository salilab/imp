/**
 *  \file JunctionTree.h
 *  \brief Stores a junction tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_JUNCTION_TREE_H
#define IMPDOMINO_JUNCTION_TREE_H

#include "config.h"
#include "IMP/base_types.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

IMPDOMINO_BEGIN_NAMESPACE

//! Stores a junction tree
/**
\note A junction tree T of an arbitrary graph G is a tree decomposition that has
these three properties:
 1. singly connected: there is exactly one path between each pair of subsets.
 2. covering: for each clique A of G there is some subset C such that A ⊆ C.
 3. running intersection: for each pair of clusters B and C that contain i,
    each cluster on the unique path between B and C also contains i.
 Further reading:
 Jordan (1999) Learning in graphical models
 */
class IMPDOMINOEXPORT JunctionTree {
public:
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>
          Graph;
  JunctionTree() {}
  JunctionTree(int number_of_nodes) {
    set_nodes(number_of_nodes);
  }

  //! Initialize a graph with N nodes
  void set_nodes(int number_of_nodes);

  void add_edge(int v1,int v2) {
    IMP_INTERNAL_CHECK(static_cast<unsigned int>(v1) < boost::num_vertices(g_),
               "input node index (" << v1 << ") is out of range ("
               << boost::num_vertices(g_) << std::endl);
    IMP_INTERNAL_CHECK(static_cast<unsigned int>(v2) < boost::num_vertices(g_),
               "input node index (" << v2 << ") is out of range ("
               << boost::num_vertices(g_) << std::endl);
    boost::add_edge(v1,v2,g_);
  }
  void set_node_name(int vi, const std::string &name) {
    IMP_USAGE_CHECK(static_cast<unsigned int>(vi) < boost::num_vertices(g_),
              "input node index (" << vi << ") is out of range ("
              << boost::num_vertices(g_) << ")"<<std::endl,ValueException);
    data_[vi].push_back(name);
  }
  const Graph *get_graph() const {return &g_;}
  int get_number_of_components(int vi) const {return data_[vi].size();}

  const std::string get_component_name(int vi,int ci) const {
    IMP_USAGE_CHECK(static_cast<unsigned int>(vi) < boost::num_vertices(g_),
              "input node index (" << vi << ") is out of range ("
              << boost::num_vertices(g_) <<")" <<std::endl,ValueException);
    IMP_USAGE_CHECK(ci < get_number_of_components(vi),
              "input component index is out of range",ValueException);
    return data_[vi][ci];
  }

  void set_component_name(int vi,int ci,const std::string &name)  {
    IMP_USAGE_CHECK(static_cast<unsigned int>(vi) < boost::num_vertices(g_),
              "input node index (" << vi << ") is out of range ("
              << boost::num_vertices(g_) <<")" <<std::endl,ValueException);
    IMP_USAGE_CHECK(ci < get_number_of_components(vi),
              "input component index is out of range",ValueException);
    data_[vi][ci]=name;
  }

  int get_number_of_nodes() const {return boost::num_vertices(g_);}
  void add_component_to_node(int vi, const std::string &name) {
     data_[vi].push_back(name);}
  bool has_edge(int n1,int n2) const{
    bool found;
    boost::graph_traits <Graph>::edge_descriptor e;
    boost::tie(e, found) =
         boost::edge(boost::vertex(n1,g_), boost::vertex(n2,g_), g_);
    return found;
  }
  void show(std::ostream& out = std::cout) const;
protected:
  typedef std::vector<std::vector<std::string> > NodeData;
  Graph g_;
  NodeData data_;
};


IMPDOMINOEXPORT void read_junction_tree(
         const std::string &filename, JunctionTree *jt);

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_JUNCTION_TREE_H */
