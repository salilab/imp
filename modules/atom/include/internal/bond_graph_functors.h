/**
 *  \file atom/internal/bond_graph_functors.h
 *  \brief Contains decorators for a bond
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H
#define IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H

#include "../config.h"
#include <IMP/core/ListSingletonContainer.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>


IMPATOM_BEGIN_INTERNAL_NAMESPACE

typedef std::pair<BondedDecorator, BondedDecorator> EdgeDescriptor;

struct MakeBonded {
  typedef Particle *argument_type;
  typedef BondedDecorator result_type;
  result_type operator()(argument_type p) const {
    return result_type(p);
  }
};


struct MakeOutEdgeDescriptor {
  BondedDecorator v_;
  MakeOutEdgeDescriptor(){}
  MakeOutEdgeDescriptor(BondedDecorator v):v_(v){}
  typedef EdgeDescriptor  result_type;
  result_type operator()(BondDecorator d) const {
    return std::make_pair(v_,
                          d.get_bonded(0) == v_
                          ? d.get_bonded(1): d.get_bonded(0));
  }
};
struct MakeInEdgeDescriptor {
  BondedDecorator v_;
  MakeInEdgeDescriptor(){}
  MakeInEdgeDescriptor(BondedDecorator v):v_(v){}
  typedef EdgeDescriptor result_type;
  result_type operator()(BondDecorator d) const {
    return std::make_pair(
                          d.get_bonded(0) == v_
                          ? d.get_bonded(1): d.get_bonded(0),
                          v_);
  }
};

struct NestedTraits {
  typedef BondedDecorator::BondIterator Inner;
  typedef core::ListSingletonContainer::ParticleIterator Outer;
  struct Get_inner {
    std::pair<Inner, Inner> operator()(Outer out) const {
      BondedDecorator bd(*out);
      return std::make_pair(bd.bonds_begin(),
                            bd.bonds_end());
    }
  };
  struct Make_value {
    typedef EdgeDescriptor result_type;
    result_type operator()(Outer out, Inner in) const {
      return std::make_pair(in->get_bonded(0),
                            in->get_bonded(1));
    }
  };
  typedef EdgeDescriptor value_type;
};




template <class Key, class Value>
struct AttributeVertexPropertyMap {
  Key k_;
  AttributeVertexPropertyMap(){}
  AttributeVertexPropertyMap(Key k): k_(k){}
  typedef BondedDecorator key_type;
  typedef Value value_type;
  struct category: public virtual boost::read_write_property_map_tag{};
  typedef value_type reference;

  value_type operator[](key_type d) const {
    return typename AttributeVertexPropertyMap<Key, Value>
      ::value_type(d.get_particle()->get_value(k_));

  }
};

template <class Key, class Value>
typename AttributeVertexPropertyMap<Key, Value>::value_type
get(const AttributeVertexPropertyMap<Key, Value> &m,
    BondedDecorator d) {
  return m[d];
}


template <class Key, class Value>
void put(const AttributeVertexPropertyMap<Key, Value> &m,
         BondedDecorator d,
         Value    v) {
  if (d.get_particle()->has_attribute(m.k_)) {
      d.get_particle()->set_value(m.k_, v);
  } else {
    d.get_particle()->add_attribute(m.k_, v);
  }
}

template <class Key>
void put(const AttributeVertexPropertyMap<Key, BondedDecorator> &m,
         BondedDecorator d,
         BondedDecorator    v) {
  if (d.get_particle()->has_attribute(m.k_)) {
    d.get_particle()->set_value(m.k_, v.get_particle());
  } else {
    d.get_particle()->add_attribute(m.k_, v.get_particle());
  }
}




template <class Key, class Value>
struct AttributeEdgePropertyMap {
  AttributeEdgePropertyMap(){}
  Key k_;
  AttributeEdgePropertyMap(Key k): k_(k){}
  typedef EdgeDescriptor key_type;
  typedef Value value_type;
  struct category: public virtual boost::read_write_property_map_tag{};
  typedef value_type reference;
  value_type operator[](key_type d) const {
    BondDecorator bd= get_bond(BondedDecorator(d.first),
                               BondedDecorator(d.second));
    return typename AttributeEdgePropertyMap<Key, Value>
      ::value_type(bd.get_particle()->get_value(k_));
  }
};

template <class Key, class Value>
typename AttributeEdgePropertyMap<Key, Value>::value_type
get(const AttributeEdgePropertyMap<Key, Value> &m,
    EdgeDescriptor d) {
  return m[d];
}


template <class Key, class Value>
void put(const AttributeVertexPropertyMap<Key, Value> &m,
         EdgeDescriptor d,
         Value    v) {
  BondDecorator bd= get_bond(BondedDecorator(d.first),
                             BondedDecorator(d.second));
  if (!bd.get_particle()->has_attribute(m.k_)) {
    bd.get_particle()->add_attribute(m.k_, v);
  } else {
    bd.get_particle()->set_value(m.k_, v);
  }
}

IMPATOM_END_INTERNAL_NAMESPACE


#endif  /* IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H */
