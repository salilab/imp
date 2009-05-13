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
struct AttributePropertyMap {
  Key k_;
  AttributePropertyMap(Key k): k_(k){}
  typedef Particle* key_type;
  typedef typename KeyAttributeType<Key>::Type value_type;
  struct category: public virtual boost::readable_property_map_tag,
                   public virtual boost::writable_property_map_tag,
                   public virtual boost::read_write_property_map_tag{};
  typedef value_type reference;
};

template <class Key, class Decorator>
typename KeyAttributeType<Key>::Type get(const AttributePropertyMap<Key> &m,
                                       Decorator d) {
  return d.get_particle()->get_value(m.k_);
}

template <class Key, class Decorator>
void put(const AttributePropertyMap<Key> &m,
         Decorator d,
         typename KeyAttributeType<Key>::Type    v) {
  return d.get_particle()->set_value(m.k_, v);
}

template <class Key>
AttributePropertyMap<Key> make_attribute_property_map(Key k) {
  return AttributePropertyMap<Key>(k);
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
  /*bool isthere=boost::isomorphism(a,b,
    boost::isomorphism_map(make_attribute_property_map(pk)),
    boost::vertex_index1_map(make_attribute_property_map(index)),
    boost::vertex_index2_map(make_attribute_property_map(index)));*/
}

IMPATOM_END_NAMESPACE
