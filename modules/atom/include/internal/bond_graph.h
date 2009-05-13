/**
 *  \file atom/internal/bond_graph.h     \brief Contains decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_BOND_GRAPH_H
#define IMPATOM_INTERNAL_BOND_GRAPH_H


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>


IMPATOM_BEGIN_NAMESPACE


inline BondedDecorator source(BondGraph::edge_descriptor bd,
                              const BondGraph &){
  return bd.first;
}


inline BondedDecorator target(const BondGraph::edge_descriptor &bd,
                                  const BondGraph &){
  return bd.second;
}

inline
std::pair<BondGraph::out_edge_iterator,
          BondGraph::out_edge_iterator>
out_edges(BondedDecorator bd,
          const BondGraph &) {
  return std::make_pair(BondGraph::out_edge_iterator(bd.bonds_begin(),
                         BondGraph::MakeOutEdgeDescriptor(bd)),
                        BondGraph::out_edge_iterator(bd.bonds_end(),
                         BondGraph::MakeOutEdgeDescriptor(bd)));
}

inline
std::pair<BondGraph::in_edge_iterator,
          BondGraph::in_edge_iterator>
in_edges(BondedDecorator bd,
          const BondGraph &) {
  return std::make_pair(BondGraph::in_edge_iterator(bd.bonds_begin(),
                         BondGraph::MakeInEdgeDescriptor(bd)),
                        BondGraph::in_edge_iterator(bd.bonds_end(),
                         BondGraph::MakeInEdgeDescriptor(bd)));
}


inline
std::pair<BondedDecorator::BondedIterator,
          BondedDecorator::BondedIterator>
adjacent_vertices(BondedDecorator bd,
                  const BondGraph &) {
  return std::make_pair(bd.bondeds_begin(),
                        bd.bondeds_end());
}

inline
std::pair<BondGraph::vertex_iterator,
          BondGraph::vertex_iterator>
vertices(const BondGraph &g) {
  core::ListSingletonContainer *lsc
    = const_cast<core::ListSingletonContainer*>(g.sc_.get());
  return
    std::make_pair(BondGraph::vertex_iterator(lsc->particles_begin(),
                                              BondGraph::MakeBonded()),
                   BondGraph::vertex_iterator(lsc->particles_end(),
                                              BondGraph::MakeBonded()));
}

inline unsigned int num_vertices(const BondGraph &g) {
  return g.sc_->get_number_of_particles();
}

inline std::pair<BondGraph::edge_iterator,
          BondGraph::edge_iterator>
edges(const BondGraph &g) {
  return
    std::make_pair(BondGraph::edge_iterator(vertices(g).first,
                                            vertices(g).second),
                   BondGraph::edge_iterator(vertices(g).second,
                                            vertices(g).second));
}

inline unsigned int
num_edges(const BondGraph &g) {
  unsigned int tot=0;
  for (BondGraph::vertex_iterator it= vertices(g).first;
       it != vertices(g).second;
       ++it) {
    tot+= it->get_number_of_bonds();
  }
  return tot;
}

inline unsigned int degree(BondedDecorator bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

inline unsigned int out_degree(BondedDecorator bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

inline unsigned int in_degree(BondedDecorator bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

IMPATOM_END_NAMESPACE


#endif  /* IMPATOM_INTERNAL_BOND_GRAPH_H */
