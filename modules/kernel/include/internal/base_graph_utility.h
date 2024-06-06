/**
 *  \file internal/base_graph_utility.h
 *  \brief Various useful utilities for working with graphs.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_BASE_GRAPH_UTILITY_H
#define IMPKERNEL_INTERNAL_BASE_GRAPH_UTILITY_H

#include <IMP/kernel_config.h>
#include "../file.h"
#include <cctype>
#include <algorithm>
#include <sstream>
#include <type_traits>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_map/property_map.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
namespace OWN {
using std::is_pointer;

template <class Graph, class ShowFunction>
class ObjectNameWriter {
  ShowFunction f_;
  typedef typename boost::property_map<Graph, boost::vertex_name_t>::const_type
      VertexMap;
  VertexMap om_;

 public:
  ObjectNameWriter(ShowFunction f, const Graph &g)
      : f_(f), om_(boost::get(boost::vertex_name, g)) {}
  void operator()(std::ostream &out, int v) const {
    std::ostringstream oss;
    f_(boost::get(om_, v), oss);
    // oss << "\\n[" << boost::get(om_, v)->get_type_name() << "]";
    std::string nm = oss.str();
    std::vector<char> vnm(nm.begin(), nm.end());
    std::string cleaned =
        std::string(vnm.begin(), std::remove(vnm.begin(), vnm.end(), '\"'));
    out << "[label=\"" << cleaned << "\"]";
  }
};
}

template <class Graph, class ShowFunction>
inline void show_as_graphviz(const Graph &g, ShowFunction f, TextOutput out) {
  OWN::ObjectNameWriter<Graph, ShowFunction> onw(f, g);
  boost::write_graphviz(out, g, onw);
}

template <class Graph, class VertexName, class VertexDescriptor,
          class GraphTraits>
inline boost::unordered_map<VertexName, VertexDescriptor>
get_graph_vertex_index(const Graph &g) {
  boost::unordered_map<VertexName, VertexDescriptor> ret;
  typename boost::property_map<Graph, boost::vertex_name_t>::const_type vm =
      boost::get(boost::vertex_name, g);
  std::pair<typename GraphTraits::vertex_iterator,
            typename GraphTraits::vertex_iterator> be = boost::vertices(g);
  for (; be.first != be.second; ++be.first) {
    VertexName vn = vm[*be.first];
    ret[vn] = *be.first;
  }
  return ret;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_BASE_GRAPH_UTILITY_H */
