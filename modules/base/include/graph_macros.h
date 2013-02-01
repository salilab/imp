/**
 *  \file IMP/base/graph_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_GRAPH_MACROS_H
#define IMPBASE_GRAPH_MACROS_H
#include <IMP/base/base_config.h>
#include <boost/graph/adjacency_list.hpp>
#include <IMP/base/map.h>
#include <boost/version.hpp>

#if defined(IMP_DOXYGEN)
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation. Directionality should be one of
    - \c bidirectional
    - \c directed
    - \c undirected
 */
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)           \
  /** See \ref graphs "Graphs in IMP" for more information.*/           \
  typedef boost::graph Name;                                            \
  typedef Name::VertexNameMap Name##ConstVertexName;                    \
  typedef Name::EdgeNameMap  Name##ConstEdgeName;                       \
  typedef boost::graph_traits<Name> Name##Traits;                       \
  typedef Name::vertex_descriptor Name##Vertex;                         \
  typedef Name::edge_descriptor Name##Edge;                             \
  class Name##VertexIndex{};                                            \
  Name##VertexIndex get_vertex_index(const Name &g)

#elif defined(SWIG)
#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ == 2 \
  && BOOST_VERSION <= 104800
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)  \
  class Name;                                                  \
  class Name##VertexIndex {}

#else // GCC VERSION
#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)  \
  class Name;                                                  \
  class Name##VertexIndex {};                                  \
  inline Name##VertexIndex get_vertex_index(const Name &g)
  #endif // GCC VERSION

#else // swig and doxygen

// Some combinations of gcc/boost fail to compile Python wrappers for
// get_vertex_index ("no match for 'operator=' error); fall back to
// std::map in this case
#if defined(__GNUC__) && __GNUC__ == 4 \
  && __GNUC_MINOR__ == 7 && BOOST_VERSION == 104800
#define IMP_GRAPH_MAP_TYPE std::map
#else
#define IMP_GRAPH_MAP_TYPE base::map
#endif

#define IMP_GRAPH(Name, directionality, VertexData, EdgeData)           \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
                                boost::directionality##S,               \
  boost::property<boost::vertex_name_t, VertexData>,                    \
  boost::property<boost::edge_name_t,                                   \
                  EdgeData> > Name;                                     \
  typedef boost::property_map<Name, boost::vertex_name_t>::const_type   \
  Name##ConstVertexName;                                                \
  typedef boost::property_map<Name, boost::edge_name_t>::const_type     \
  Name##ConstEdgeName;                                                  \
  typedef boost::graph_traits<Name> Name##Traits;                       \
  typedef Name##Traits::vertex_descriptor Name##Vertex;                 \
  typedef Name##Traits::edge_descriptor Name##Edge;                     \
  typedef IMP_GRAPH_MAP_TYPE<VertexData, Name##Vertex> Name##VertexIndex; \
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
  typedef boost::property_map<Name, boost::edge_name_t>::type           \
  Name##EdgeName;                                                       \
  typedef boost::property_map<Name, boost::vertex_name_t>::type         \
  Name##VertexName
#endif // swig and doxygen


#ifdef IMP_DOXYGEN
//! Define a graph object in \imp
/** The docs for the graph should appear before the macro
    invocation. Directionality should be one of
    - \c bidirectional
    - \c directed
    - \c undirected
 */
#define IMP_WEIGHTED_GRAPH(Name, directionality, VertexData)    \
  /** See \ref graphs "Graphs" for more information.*/  \
  typedef boost::graph Name

#elif defined(SWIG)
#define IMP_WEIGHTED_GRAPH(Name, directionality, VertexData)  class Name
#else
#define IMP_WEIGHTED_GRAPH(Name, directionality, VertexData)            \
  typedef boost::adjacency_list<boost::vecS, boost::vecS,               \
                                boost::directionality##S,               \
  boost::property<boost::vertex_name_t, VertexData>,                    \
                                boost::property<boost::edge_weight_t,   \
                  double> > Name;                                       \
  typedef boost::property_map<Name, boost::vertex_name_t>::const_type   \
  Name##ConstVertexName;                                                \
  typedef boost::property_map<Name, boost::edge_weight_t>::const_type   \
  Name##ConstEdgeWeight;                                                \
  typedef boost::graph_traits<Name> Name##Traits;                       \
  typedef Name##Traits::vertex_descriptor Name##Vertex;                 \
  typedef Name##Traits::edge_descriptor Name##Edge;                     \
  typedef base::map<VertexData, Name##Vertex> Name##VertexIndex; \
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
  typedef boost::property_map<Name, boost::edge_weight_t>::type         \
  Name##EdgeWeight;                                                     \
  typedef boost::property_map<Name, boost::vertex_name_t>::type         \
  Name##VertexName
#endif


#endif  /* IMPBASE_GRAPH_MACROS_H */
