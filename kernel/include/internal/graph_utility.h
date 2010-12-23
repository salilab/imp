/**
 *  \file internal/graph_utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_GRAPH_UTILITY_H
#define IMP_INTERNAL_GRAPH_UTILITY_H

#include "../kernel_config.h"
#include "../Particle.h"
#include "map.h"
#include <cctype>
#include <algorithm>
#include <sstream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


IMP_BEGIN_INTERNAL_NAMESPACE

template <class Graph>
class ObjectNameWriter {
  typedef typename boost::property_map<Graph,
                          boost::vertex_name_t>::const_type VertexMap;
  VertexMap om_;
  template <class T>
  std::string get_name(const T&t) const {return t.get_name();}
  template <class T>
  std::string get_name(T* t) const { return t->get_name();}
  template <class T>
  std::string get_name(Pointer<T> t) const { return t->get_name();}
public:
  ObjectNameWriter( const Graph&g): om_(boost::get(boost::vertex_name,g)){}
  void operator()(std::ostream& out, int v) const {
    std::string nm=get_name(boost::get(om_, v));
    std::vector<char> vnm(nm.begin(), nm.end());
    out << "[label=\""
        << std::string(vnm.begin(), std::remove(vnm.begin(), vnm.end(),
                                                '\"')) << "\"]";
  }
};

template <class Graph>
void show_as_graphviz(const Graph &g, std::ostream &out) {
  ObjectNameWriter<Graph> onw(g);
  boost::write_graphviz(out, g, onw);
}

template <class Base, class Graph>
Map<Base*, int> get_graph_index(const Graph &g) {
  Map<Base*, int>ret;
  typename boost::property_map<Graph,
                               boost::vertex_name_t>::const_type
    vm= boost::get(boost::vertex_name,g);
  for (unsigned int i=0; i< boost::num_vertices(g); ++i) {
    Object *o= vm[i];
    if (dynamic_cast<Base*>(o)) {
      ret[dynamic_cast<Base*>(o)]= i;
    }
  }
  return ret;
}




struct AncestorException{
  Object *o;
  AncestorException(Object *oi): o(oi){};
};

// gcc 4.2 objects if this does not have external linkage
template <class Graph>
class AncestorVisitor: public boost::default_dfs_visitor {
  std::set<Particle*> pst_;
  typename boost::property_map<Graph,
                               boost::vertex_name_t>::const_type vm_;
public:
  AncestorVisitor(){}
  AncestorVisitor(const ParticlesTemp& pst,
                  const Graph&g): pst_(pst.begin(), pst.end()),
                                  vm_(boost::get(boost::vertex_name, g)){}
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& g) {
    Object *o= vm_[u];
    //std::cout << "Visiting " << o->get_name() << std::endl;
    if (pst_.find(dynamic_cast<Particle*>(o)) != pst_.end()) {
      throw AncestorException(o);
    }
  }
  /*template <class Edge>
  void tree_edge(Edge e, const Graph &g) {
    typename boost::graph_traits<Graph>::vertex_descriptor s=
      = boost::source(e, g);
    typename boost::graph_traits<Graph>::vertex_descriptor t
      = boost::target(e, g);
    std::cout << "Tree edge " << vm_[s]->get_name()
      << "->" << vm_[t]->get_name() << std::endl;
      }*/
};

template <class G>
bool get_has_ancestor(const G &g,
                      unsigned int v,
                      const ParticlesTemp &pst) {
  typedef boost::reverse_graph<G>  RG;
  RG rg(g);
  AncestorVisitor<RG> av(pst,g);
  boost::vector_property_map<int> color(boost::num_vertices(g));
  try {
    //std::cout << "Searching for dependents of " << v << std::endl;
    boost::depth_first_visit(rg, v, av, color);
    return false;
  } catch (AncestorException e) {
    /*IMP_LOG(VERBOSE, "Vertex has ancestor \"" << e.o->get_name()
      << "\"" << std::endl);*/
    return true;
  }
}

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_GRAPH_UTILITY_H */
