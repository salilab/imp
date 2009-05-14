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
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/internal/NestedIterator.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/transform_iterator.hpp>

#ifndef IMP_SWIG

IMPATOM_BEGIN_NAMESPACE


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
   \untested{BondGraph}
 */
class IMPATOMEXPORT BondGraph: public NullDefault{


public:
#ifndef IMP_DOXYGEN
  struct MakeBonded {
    typedef Particle *argument_type;
    typedef BondedDecorator result_type;
    result_type operator()(argument_type p) const {
      return result_type(p);
    }
  };
#endif
  IMP_NO_DOXYGEN(Pointer<core::ListSingletonContainer> sc_;)

  //! The graph is on the leaves of the MolecularHierachyDecorator
  /** All the leaves will be made Bonded particles if they are not already.
   */
  BondGraph(MolecularHierarchyDecorator bd);

  BondGraph(){}

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

  struct MakeOutEdgeDescriptor {
    BondedDecorator v_;
    MakeOutEdgeDescriptor(){}
    MakeOutEdgeDescriptor(BondedDecorator v):v_(v){}
    typedef edge_descriptor result_type;
    edge_descriptor operator()(BondDecorator d) const {
      return std::make_pair(v_,
                            d.get_bonded(0) == v_
                            ? d.get_bonded(1): d.get_bonded(0));
    }
  };
  struct MakeInEdgeDescriptor {
    BondedDecorator v_;
    MakeInEdgeDescriptor(){}
    MakeInEdgeDescriptor(BondedDecorator v):v_(v){}
    typedef edge_descriptor result_type;
    edge_descriptor operator()(BondDecorator d) const {
      return std::make_pair(
                            d.get_bonded(0) == v_
                            ? d.get_bonded(1): d.get_bonded(0),
                            v_);
    }
  };


  typedef boost::transform_iterator<MakeOutEdgeDescriptor,
    vertex_descriptor::BondIterator> out_edge_iterator;
  typedef vertex_descriptor::BondedIterator adjacency_iterator;
  typedef boost::transform_iterator<MakeBonded,
                   IMP::core::ListSingletonContainer::ParticleIterator>
  vertex_iterator;
  typedef boost::disallow_parallel_edge_tag edge_parallel_category;
  typedef boost::undirected_tag directed_category;

  typedef boost::transform_iterator<MakeInEdgeDescriptor,
    vertex_descriptor::BondIterator> in_edge_iterator;



  struct NestedTraits {
    typedef BondedDecorator::BondIterator Inner;
    typedef vertex_iterator Outer;
    struct Get_inner {
      std::pair<Inner, Inner> operator()(Outer out) const {
        return std::make_pair(out->bonds_begin(),
                              out->bonds_end());
      }
    };
    struct Make_value {
      typedef edge_descriptor result_type;
      edge_descriptor operator()(Outer out, Inner in) const {
        return std::make_pair(in->get_bonded(0),
                              in->get_bonded(1));
      }
    };
    typedef edge_descriptor value_type;
  };

  typedef IMP::internal::NestedIterator<NestedTraits> edge_iterator;

  struct graph_tag{};
  typedef Int vertex_property_type;
#endif
};

IMPATOM_END_NAMESPACE

#include "internal/bond_graph.h"

#endif // IMP_SWIG

#endif  /* IMPATOM_BOND_GRAPH_H */
