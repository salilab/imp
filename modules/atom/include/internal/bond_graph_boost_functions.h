/**
 *  \file atom/internal/bond_graph_boost_functions.h
 *  \brief Contains decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_BOND_GRAPH_BOOST_FUNCTIONS_H
#define IMPATOM_INTERNAL_BOND_GRAPH_BOOST_FUNCTIONS_H


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>


IMPATOM_BEGIN_NAMESPACE


inline Bonded source(BondGraph::edge_descriptor bd,
                              const BondGraph &){
  return bd.first;
}


inline Bonded target(const BondGraph::edge_descriptor &bd,
                                  const BondGraph &){
  return bd.second;
}

inline
std::pair<BondGraph::out_edge_iterator,
          BondGraph::out_edge_iterator>
out_edges(Bonded bd,
          const BondGraph &) {
  return std::make_pair(BondGraph::out_edge_iterator(bd.bonds_begin(),
                             internal::MakeOutEdgeDescriptor(bd)),
                        BondGraph::out_edge_iterator(bd.bonds_end(),
                             internal::MakeOutEdgeDescriptor(bd)));
}

inline
std::pair<BondGraph::in_edge_iterator,
          BondGraph::in_edge_iterator>
in_edges(Bonded bd,
          const BondGraph &) {
  return std::make_pair(BondGraph::in_edge_iterator(bd.bonds_begin(),
                           internal::MakeInEdgeDescriptor(bd)),
                        BondGraph::in_edge_iterator(bd.bonds_end(),
                           internal::MakeInEdgeDescriptor(bd)));
}


inline
std::pair<Bonded::BondedIterator,
          Bonded::BondedIterator>
adjacent_vertices(Bonded bd,
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
                                              internal::MakeBonded()),
                   BondGraph::vertex_iterator(lsc->particles_end(),
                                              internal::MakeBonded()));
}

inline unsigned int num_vertices(const BondGraph &g) {
  return g.sc_->get_number_of_particles();
}

inline std::pair<BondGraph::edge_iterator,
          BondGraph::edge_iterator>
edges(const BondGraph &g) {
  core::ListSingletonContainer *sc
    = const_cast<core::ListSingletonContainer*>(g.sc_.get());
  return
    std::make_pair(BondGraph::edge_iterator(sc->particles_begin(),
                                            sc->particles_end()),
                   BondGraph::edge_iterator(sc->particles_end(),
                                            sc->particles_end()));
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

inline unsigned int degree(Bonded bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

inline unsigned int out_degree(Bonded bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

inline unsigned int in_degree(Bonded bd,
                    const BondGraph&) {
  return bd.get_number_of_bonds();
}

IMPATOM_END_NAMESPACE


#endif  /* IMPATOM_INTERNAL_BOND_GRAPH_BOOST_FUNCTIONS_H */
