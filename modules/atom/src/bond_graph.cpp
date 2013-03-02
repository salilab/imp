/**
 *  \file bond_graph.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/bond_graph.h>
#include <boost/graph/graph_concepts.hpp>
#include <IMP/SingletonContainer.h>

#ifdef _MSC_VER
#pragma warning( disable : 4675)
#endif

#include <boost/graph/isomorphism.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_utility.hpp>
#if BOOST_VERSION >= 104000
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPATOM_BEGIN_NAMESPACE
BondGraph::BondGraph(Hierarchy bd):
  sc_(get_as<Particles>(get_leaves(bd))) {
  for (unsigned int i=0; i< sc_.size(); ++i) {
    if (!Bonded::particle_is_instance(sc_[i])) {
      Bonded::setup_particle(sc_[i]);
    }
  }
}

BondGraph::VertexIntPropertyMap BondGraph::get_vertex_index_map() const {
  if (index_key_== IntKey()) {
    std::ostringstream oss;
    oss << this << " bond graph index";
    index_key_= IntKey(oss.str().c_str());
    int last=0;
    for (Particles::const_iterator it= sc_.begin(); it != sc_.end(); ++it) {
      (*it)->add_attribute(index_key_, last);
      ++last;
    }
  }
  return VertexIntPropertyMap(index_key_);
}


BondGraph::~BondGraph() {
  if (index_key_ != IntKey()) {
    for (unsigned int i=0; i<sc_.size(); ++i) {
      sc_[i]->remove_attribute(index_key_);
    }
  }
}

namespace {
#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")
#endif
void bgl_concept_checks() {

  boost::function_requires<boost::VertexListGraphConcept<BondGraph> >();
  boost::function_requires<boost::AdjacencyGraphConcept<BondGraph> >();
  boost::function_requires<boost::EdgeListGraphConcept<BondGraph> >();
  boost::function_requires<boost::IncidenceGraphConcept<BondGraph> >();
  boost::function_requires<boost::BidirectionalGraphConcept<BondGraph> >();

  BondGraph a,b;
  ParticleIndexKey pk("isomaping");
  /*boost::isomorphism(a,b,
         boost::isomorphism_map(BondGraph::VertexVertexPropertyMap(pk))
                     .vertex_index1_map(a.get_vertex_index_map())
                     .vertex_index2_map(b.get_vertex_index_map()));*/

  boost::dijkstra_shortest_paths(a, Bonded(),
        boost::predecessor_map(BondGraph::VertexVertexPropertyMap(pk))
     .weight_map(BondGraph::EdgeFloatPropertyMap(FloatKey("bond length")))
     .distance_map(BondGraph::VertexFloatPropertyMap(FloatKey("hi")))
     .vertex_index_map(a.get_vertex_index_map()));
}
}

IMPATOM_END_NAMESPACE
