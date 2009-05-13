/**
 *  \file bond_graph.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/bond_graph.h>
#include <boost/graph/graph_concepts.hpp>
#include <IMP/SingletonContainer.h>

IMPATOM_BEGIN_NAMESPACE
BondGraph::BondGraph(MolecularHierarchyDecorator bd):
  sc_(new core::ListSingletonContainer(get_leaves(bd))) {
  for (core::ListSingletonContainer::ParticleIterator
         it= sc_->particles_begin();
       it != sc_->particles_end(); ++it) {
    if (!BondedDecorator::is_instance_of(*it)) {
      BondedDecorator::create(*it);
    }
  }
}

void bgl_concept_checks() {
  boost::function_requires<boost::VertexListGraphConcept<BondGraph> >();
  boost::function_requires<boost::AdjacencyGraphConcept<BondGraph> >();
  boost::function_requires<boost::EdgeListGraphConcept<BondGraph> >();
  boost::function_requires<boost::IncidenceGraphConcept<BondGraph> >();
  boost::function_requires<boost::BidirectionalGraphConcept<BondGraph> >();
}

IMPATOM_END_NAMESPACE
