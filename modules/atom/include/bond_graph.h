/**
 *  \file atom/bond_graph.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_BOND_GRAPH_H
#define IMPATOM_BOND_GRAPH_H

#include "bond_decorators.h"
#include "MolecularHierarchyDecorator.h"
#include "internal/bond_graph_functors.h"
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/internal/NestedIterator.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/transform_iterator.hpp>

#ifndef IMP_SWIG

IMPATOM_BEGIN_NAMESPACE


#ifndef IMP_DOXYGEN

typedef internal::AttributeVertexPropertyMap<FloatKey, Float>
VertexFloatPropertyMap;
typedef internal::AttributeVertexPropertyMap<IntKey, Int>
VertexIntPropertyMap;
typedef internal::AttributeVertexPropertyMap<ParticleKey, BondedDecorator>
VertexVertexPropertyMap;



typedef internal::AttributeEdgePropertyMap<FloatKey, Float>
EdgeFloatPropertyMap;
typedef internal::AttributeEdgePropertyMap<IntKey, Int> EdgeIntPropertyMap;

#else

/** @name Property maps
    Boost.Graph makes extensive use of property maps. We define a few maps
    which use the particle attributes to store their data. Each map is
    constructed by passing an appropriate attribute key, to specify the
    attribute used to store the data. Attempt to store to an attribute
    which the particle does not already have adds the attribute to the
    particle.
   @{
 */
class VertexFloatPropertyMap;
class VertexIntPropertyMap;
class VertexVertexPropertyMap
class EdgeFloatPropertyMap;
class EdgeIntPropertyMap;
/** @} */
#endif


//! Represent a bond graph as a boost graph
/** The BondGraph class implements the following boost graph concepts
    - bidirectional_graph
    - adjacency_graph
    - vertex_list_graph
    - edge_list_graph.

    See the
\external{www.boost.org/doc/libs/1_39_0/libs/graph/doc/table_of_contents.html,
    Boost.Graph manual} for more details and a list of algorithms.

    The vertices are BondedDecorators and the edge descriptors are
    \code
    std::pair<BondedDecorator, BondededDecorator>
    \endcode.

    And all vertices can be assumed to have boost vertex indices.

    Some usage examples
    \code
    boost::isomorphism(a,b,
                     boost::isomorphism_map(VertexVertexPropertyMap(pk))
                     .vertex_index1_map(a.get_vertex_index_map())
                     .vertex_index2_map(b.get_vertex_index_map()));
    boost::dijkstra_shortest_paths(a, BondedDecorator(),
        boost::predecessor_map(VertexVertexPropertyMap(pk))
           .weight_map(EdgeFloatPropertyMap(FloatKey("bond length")))
           .distance_map(VertexFloatPropertyMap(FloatKey("hi")))
           .vertex_index_map(VertexIntPropertyMap(index)));
    \endcode
   \untested{BondGraph}
 */
class IMPATOMEXPORT BondGraph: public NullDefault{

  mutable IntKey index_key_;

public:
  IMP_NO_DOXYGEN(Pointer<core::ListSingletonContainer> sc_;)

  //! The graph is on the leaves of the MolecularHierachyDecorator
  /** All the leaves will be made Bonded particles if they are not already.
   */
  BondGraph(MolecularHierarchyDecorator bd);

  BondGraph(){}

  ~BondGraph();

  VertexIntPropertyMap get_vertex_index_map() const;


#ifndef IMP_DOXYGEN
  struct traversal_category: public virtual boost::adjacency_graph_tag,
                             public virtual boost::vertex_list_graph_tag,
                             public virtual boost::edge_list_graph_tag,
    //public virtual boost::incidence_graph_tag
                             public virtual boost::bidirectional_graph_tag
  {};
  typedef BondedDecorator vertex_descriptor;
  typedef
    std::pair<BondedDecorator, BondedDecorator> edge_descriptor;
  //typedef undirected_tag directed_category;
  typedef int vertices_size_type;
  typedef int edges_size_type;
  typedef int degree_size_type;

  typedef boost::transform_iterator<internal::MakeOutEdgeDescriptor,
    vertex_descriptor::BondIterator> out_edge_iterator;
  typedef vertex_descriptor::BondedIterator adjacency_iterator;
  typedef boost::transform_iterator<internal::MakeBonded,
                   IMP::core::ListSingletonContainer::ParticleIterator>
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
