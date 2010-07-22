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
#include <boost/format.hpp>
#include <algorithm>
#include <sstream>
#include <boost/graph/graphviz.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE

template <class Graph>
class ObjectNameWriter {
  typedef typename boost::property_map<Graph,
                                       boost::vertex_name_t>::type VertexMap;
  VertexMap om_;
  template <class T>
  std::string get_name(const T&t) const {return t.get_name();}
  template <class T>
  std::string get_name(T* t) const { return t->get_name();}
  template <class T>
  std::string get_name(Pointer<T> t) const { return t->get_name();}
public:
  ObjectNameWriter( const Graph&g): om_(boost::get(boost::vertex_name,
                                                   const_cast<Graph&>(g))){}
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

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_GRAPH_UTILITY_H */
