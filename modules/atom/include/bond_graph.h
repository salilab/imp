/**
 *  \file IMP/atom/bond_graph.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_BOND_GRAPH_H
#define IMPATOM_BOND_GRAPH_H

#include "bond_decorators.h"
#include "Hierarchy.h"
#include "internal/bond_graph_functors.h"
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/internal/NestedIterator.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/transform_iterator.hpp>

#ifndef SWIG

IMPATOM_BEGIN_NAMESPACE

//! Represent a bond graph as a boost graph
/** The BondGraph class implements the following boost graph concepts
    - bidirectional_graph
    - adjacency_graph
    - vertex_list_graph
    - edge_list_graph.

    See the
\external{http://www.boost.org/doc/libs/1_39_0/libs/graph/doc/table_of_contents.html,
    Boost.Graph manual} for more details and a list of algorithms.

    The vertices are Bondeds. The type of the edge_descriptors
    is internal. You can use the get_bond() method to get the corresponding
    Bond.

    And all vertices can be assumed to have internal boost vertex indices.
    The Boost.PropertyMap to access these can be accessed using the
    get_vertex_index_map() function.

    Some usage examples
    \code
    boost::isomorphism(a,b,
        boost::isomorphism_map(BondGraph::VertexVertexPropertyMap(pk))
        .vertex_index1_map(a.get_vertex_index_map())
        .vertex_index2_map(b.get_vertex_index_map()));
boost::dijkstra_shortest_paths(a, Bonded(),
 boost::predecessor_map(BondGraph::VertexVertexPropertyMap(pk))
 .weight_map(BondGraph::EdgeFloatPropertyMap(Bond::get_legnth_key()))
 .distance_map(BondGraph::VertexFloatPropertyMap(FloatKey("graph distance")))
 .vertex_index_map(a.get_vertex_index_map()));
    \endcode

   \untested{BondGraph}
 */
class IMPATOMEXPORT BondGraph{

  mutable IntKey index_key_;

public:
  IMP_NO_DOXYGEN(mutable Particles sc_);

  //! The graph is on the leaves of the atom::Hierarchy
  /** All the leaves will be made Bonded particles if they are not already.
      \note The hierarchy must not have any bonds to particles not in the
      hierarchy.
   */
  BondGraph(Hierarchy bd);

  BondGraph(){}

  ~BondGraph();

#ifndef IMP_DOXYGEN

  typedef internal::AttributeVertexPropertyMap<FloatKey, Float>
    VertexFloatPropertyMap;
  typedef internal::AttributeVertexPropertyMap<IntKey, Int>
    VertexIntPropertyMap;
  typedef internal::AttributeVertexPropertyMap<ParticleIndexKey, Bonded>
    VertexVertexPropertyMap;



  typedef internal::AttributeEdgePropertyMap<FloatKey, Float>
    EdgeFloatPropertyMap;
  typedef internal::AttributeEdgePropertyMap<IntKey, Int> EdgeIntPropertyMap;

  VertexIntPropertyMap get_vertex_index_map() const;
#else
  /** @name Property maps
      Boost.Graph makes extensive use of property maps. We define a few maps
      which use the particle attributes to store their data. Each map is
      constructed by passing an appropriate attribute key, to specify the
      attribute used to store the data. Attempt to store to an attribute
      which the particle does not already have adds the attribute to the
      particle. More information about property maps can be found in
\external{http://www.boost.org/doc/libs/1_39_0/libs/property_map/property_map.html,
      the Boost.PropertyMap documentation}.
      @{
  */
  //! Provide boost access to a Float attribute each vertex
  typename VertexFloatPropertyMap;
  //! Provide boost access to an Int attribute each vertex
  typename VertexIntPropertyMap;
  //! Provide boost access to a Particle attribute each vertex
  /** The particles in question must be Bonded particles. */
  typename VertexVertexPropertyMap;
  //! Provide boost access to a Float attribute on the edges such as length
  typename EdgeFloatPropertyMap;
  //! Provide boost access to an Int attribute on each edge.
  typename EdgeIntPropertyMap;

  VertexIntPropertyMap get_vertex_index_map() const;
  /** @} */
#endif


#ifndef IMP_DOXYGEN
  typedef std::pair<Bonded,
         Bonded> edge_descriptor;
#endif

  Bond get_bond(edge_descriptor d) const {
    return IMP::atom::get_bond(d.first, d.second);
  }

#ifndef IMP_DOXYGEN
  struct traversal_category: public virtual boost::adjacency_graph_tag,
                             public virtual boost::vertex_list_graph_tag,
                             public virtual boost::edge_list_graph_tag,
    //public virtual boost::incidence_graph_tag
                             public virtual boost::bidirectional_graph_tag
  {};
  typedef Bonded vertex_descriptor;
  //typedef undirected_tag directed_category;
  typedef int vertices_size_type;
  typedef int edges_size_type;
  typedef int degree_size_type;

  typedef boost::transform_iterator<internal::MakeOutEdgeDescriptor,
    vertex_descriptor::BondIterator> out_edge_iterator;
  typedef vertex_descriptor::BondedIterator adjacency_iterator;
  typedef boost::transform_iterator<internal::MakeBonded,
                                    IMP::Particles::iterator>
  vertex_iterator;
  typedef boost::disallow_parallel_edge_tag edge_parallel_category;
  typedef boost::undirected_tag directed_category;

  typedef boost::transform_iterator<internal::MakeInEdgeDescriptor,
    vertex_descriptor::BondIterator> in_edge_iterator;





  typedef IMP::internal::NestedIterator<internal::NestedTraits> edge_iterator;

  struct graph_tag{};
  typedef Int vertex_property_type;
#endif

};




IMPATOM_END_NAMESPACE

#include "internal/bond_graph_boost_functions.h"

#endif // IMP_SWIG

#endif  /* IMPATOM_BOND_GRAPH_H */
