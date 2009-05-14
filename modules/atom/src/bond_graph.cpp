/**
 *  \file bond_graph.cpp     \brief Decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/bond_graph.h>
#include <boost/graph/graph_concepts.hpp>
#include <IMP/SingletonContainer.h>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/property_map.hpp>

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

template <class Key>
struct AttributeVertexPropertyMap {
  Key k_;
  AttributeVertexPropertyMap(Key k): k_(k){}
  typedef BondedDecorator key_type;
  typedef typename KeyAttributeType<Key>::Type value_type;
  struct category: public virtual boost::readable_property_map_tag,
                   public virtual boost::writable_property_map_tag,
                   public virtual boost::read_write_property_map_tag{};
  typedef value_type reference;
};

template <class Key>
typename KeyAttributeType<Key>::Type
get(const AttributeVertexPropertyMap<Key> &m,
    BondedDecorator d) {
  return d.get_particle()->get_value(m.k_);
}

template <class Key>
typename KeyAttributeType<Key>::Type
get(const AttributeVertexPropertyMap<Key> &m,
    Particle* d) {
  return d->get_value(m.k_);
}

BondedDecorator get(const AttributeVertexPropertyMap<ParticleKey> &m,
                    BondedDecorator d) {
  return  BondedDecorator(d.get_particle()->get_value(m.k_));
}

template <class Key>
void put(const AttributeVertexPropertyMap<Key> &m,
         BondedDecorator d,
         typename KeyAttributeType<Key>::Type    v) {
  return d.get_particle()->set_value(m.k_, v);
}

template <class Key>
void put(const AttributeVertexPropertyMap<Key> &m,
         BondedDecorator d,
         BondedDecorator    v) {
  return d.get_particle()->set_value(m.k_, v.get_particle());
}

template <class Key>
void put(const AttributeVertexPropertyMap<Key> &m,
         Particle* d,
         typename KeyAttributeType<Key>::Type    v) {
  return d->set_value(m.k_, v);
}


template <class Key>
AttributeVertexPropertyMap<Key> make_attribute_property_map(Key k) {
  return AttributeVertexPropertyMap<Key>(k);
}


AttributeVertexPropertyMap<IntKey> get(BondGraph::vertex_property_type,
                                 const BondGraph &) {
  return AttributeVertexPropertyMap<IntKey>(IntKey("dummy"));
}

struct Degree {
  typedef unsigned int result_type;
  typedef BondedDecorator argument_type;
  result_type operator()(argument_type t) {
    return t.get_number_of_bonds();
  }
};

struct BondLengthMap{
  typedef BondGraph::edge_descriptor argument_type;
  typedef double result_type;
  double operator()(argument_type e) const {
    return get_bond(BondedDecorator(e.first),
                    BondedDecorator(e.second)).get_length();
  }
  typedef argument_type key_type;
  typedef double value_type;
  struct category: public virtual boost::readable_property_map_tag{};
  typedef value_type reference;
};

double get(const BondLengthMap &m,
            BondGraph::edge_descriptor d) {
  return m(d);
}



void bgl_concept_checks() {
  boost::function_requires<boost::VertexListGraphConcept<BondGraph> >();
  boost::function_requires<boost::AdjacencyGraphConcept<BondGraph> >();
  boost::function_requires<boost::EdgeListGraphConcept<BondGraph> >();
  boost::function_requires<boost::IncidenceGraphConcept<BondGraph> >();
  boost::function_requires<boost::BidirectionalGraphConcept<BondGraph> >();

  BondGraph a,b;
  IntKey index("crazy temp index");
  ParticleKey pk("isomaping");
  /*  bool isthere=boost::isomorphism(a,b,
                                  AttributeVertexPropertyMap<IntKey>(index),
                                  Degree(), Degree(),
                                  1000000,
                                  make_attribute_property_map(index),
                                  make_attribute_property_map(index));*/
  boost::dijkstra_shortest_paths(a, BondedDecorator(),
        boost::predecessor_map(AttributeVertexPropertyMap<ParticleKey>(pk))
           .weight_map(BondLengthMap())
           .distance_map(AttributeVertexPropertyMap<FloatKey>(FloatKey("hi")))
           .vertex_index_map(AttributeVertexPropertyMap<IntKey>(index)));
}

IMPATOM_END_NAMESPACE
/*,
    boost::vertex_index1_map=make_attribute_property_map(index)),
    boost::vertex_index2_map=make_attribute_property_map(index))*/
