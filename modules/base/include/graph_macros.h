/**
 *  \file IMP/base/graph_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_GRAPH_MACROS_H
#define IMPBASE_GRAPH_MACROS_H
#include "base_config.h"
#include <boost/graph/adjacency_list.hpp>
#include <IMP/compatibility/map.h>

#ifdef IMP_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation. Directionality should be one of
    - \c bidirectional
    - \c directed
    - \c undirected
 */
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)     \
  /** See \ref graphs "Graphs" for more information.*/  \
  typedef boost::graph Name

#elif defined(SWIG)
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)  class Name
#else
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)           \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
                                boost::directionality##S,               \
  boost::property<boost::vertex_name_t, VertexData>,                    \
  boost::property<boost::edge_name_t,                                   \
                  EdgeData> > Name;                                     \
  typedef boost::property_map<Name, boost::vertex_name_t>::const_type   \
  Name##ConstVertexName;                                                \
  typedef boost::graph_traits<Name> Name##Traits;                       \
  typedef Name##Traits::vertex_descriptor Name##Vertex;                 \
  typedef compatibility::map<VertexData, Name##Vertex> Name##VertexIndex; \
  inline Name##VertexIndex get_vertex_index(const Name &g) {            \
    Name##ConstVertexName vm = boost::get(boost::vertex_name, g);       \
    std::pair<Name##Traits::vertex_iterator, Name##Traits::vertex_iterator> \
      be= boost::vertices(g);                                           \
    Name##VertexIndex ret;                                              \
    for (; be.first != be.second; ++be.first) {                         \
      ret[vm[*be.first]]= *be.first;                                    \
    }                                                                   \
    return ret;                                                         \
  }                                                                     \
  typedef boost::property_map<Name, boost::vertex_name_t>::type         \
  Name##VertexName
#endif


#endif  /* IMPBASE_GRAPH_MACROS_H */
