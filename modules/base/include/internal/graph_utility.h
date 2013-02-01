/**
 *  \file internal/graph_utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_GRAPH_UTILITY_H
#define IMPBASE_INTERNAL_GRAPH_UTILITY_H

#include <IMP/base/base_config.h>
#include <IMP/base/map.h>
#include "../Vector.h"
#include "../Object.h"
#include <cctype>
#include <algorithm>
#include <sstream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <IMP/base/vector_property_map.h>


IMPBASE_BEGIN_INTERNAL_NAMESPACE
namespace OWN {
  using boost::enable_if;
  using boost::mpl::and_;
  using boost::mpl::not_;
  using boost::is_convertible;
  using boost::is_base_of;
  using boost::is_pointer;

template <class Graph>
class ObjectNameWriter {

  typedef typename boost::property_map<Graph,
                          boost::vertex_name_t>::const_type VertexMap;
  VertexMap om_;
public:
  ObjectNameWriter( const Graph&g): om_(boost::get(boost::vertex_name,g)){}
  void operator()(std::ostream& out, int v) const {
    typedef typename boost::property_traits<typename boost::property_map<Graph,
                           boost::vertex_name_t>::const_type>::value_type VT;
    std::ostringstream oss;
    oss << Showable(boost::get(om_, v));
    std::string nm=oss.str();
    base::Vector<char> vnm(nm.begin(), nm.end());
    out << "[label=\""
        << std::string(vnm.begin(), std::remove(vnm.begin(), vnm.end(),
                                                '\"')) << "\"]";
  }
};
}

template <class Graph>
inline void show_as_graphviz(const Graph &g, std::ostream &out) {
  OWN::ObjectNameWriter<Graph> onw(g);
  boost::write_graphviz(out, g, onw);
}

template <class Base, class Graph>
inline base::map<Base*, int> get_graph_index(const Graph &g) {
  base::map<Base*, int>ret;
  typename boost::property_map<Graph,
                               boost::vertex_name_t>::const_type
    vm= boost::get(boost::vertex_name,g);
  for (unsigned int i=0; i< boost::num_vertices(g); ++i) {
    base::Object *o= vm[i];
    if (dynamic_cast<Base*>(o)) {
      ret[dynamic_cast<Base*>(o)]= i;
    }
  }
  return ret;
}





IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_GRAPH_UTILITY_H */
