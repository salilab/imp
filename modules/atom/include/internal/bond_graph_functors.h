/**
 *  \file atom/internal/bond_graph_functors.h
 *  \brief Contains decorators for a bond
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H
#define IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H

#include <IMP/atom/atom_config.h>
#include <IMP/container/ListSingletonContainer.h>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

typedef std::pair<Bonded, Bonded> EdgeDescriptor;

struct MakeBonded {
  typedef Particle *argument_type;
  typedef Bonded result_type;
  result_type operator()(argument_type p) const { return result_type(p); }
};

struct MakeOutEdgeDescriptor {
  Bonded v_;
  MakeOutEdgeDescriptor() {}
  MakeOutEdgeDescriptor(Bonded v) : v_(v) {}
  typedef EdgeDescriptor result_type;
  result_type operator()(Bond d) const {
    return std::make_pair(
        v_, d.get_bonded(0) == v_ ? d.get_bonded(1) : d.get_bonded(0));
  }
};
struct MakeInEdgeDescriptor {
  Bonded v_;
  MakeInEdgeDescriptor() {}
  MakeInEdgeDescriptor(Bonded v) : v_(v) {}
  typedef EdgeDescriptor result_type;
  result_type operator()(Bond d) const {
    return std::make_pair(
        d.get_bonded(0) == v_ ? d.get_bonded(1) : d.get_bonded(0), v_);
  }
};

struct NestedTraits {
  typedef Bonded::BondIterator Inner;
  typedef Particles::iterator Outer;
  struct Get_inner {
    std::pair<Inner, Inner> operator()(Outer out) const {
      Bonded bd(*out);
      return std::make_pair(bd.bonds_begin(), bd.bonds_end());
    }
  };
  struct Make_value {
    typedef EdgeDescriptor result_type;
    result_type operator()(Outer, Inner in) const {
      return std::make_pair(in->get_bonded(0), in->get_bonded(1));
    }
  };
  typedef EdgeDescriptor value_type;
};

template <class Key, class Value>
struct AttributeVertexPropertyMap {
  Key k_;
  AttributeVertexPropertyMap() {}
  AttributeVertexPropertyMap(Key k) : k_(k) {}
  typedef Bonded key_type;
  typedef Value value_type;
  struct category : public virtual boost::read_write_property_map_tag {};
  typedef value_type reference;

  value_type operator[](key_type d) const {
    return typename AttributeVertexPropertyMap<Key, Value>::value_type(
        d.get_particle()->get_value(k_));
  }
};

template <class Key, class Value>
inline typename AttributeVertexPropertyMap<Key, Value>::value_type get(
    const AttributeVertexPropertyMap<Key, Value> &m, Bonded d) {
  return m[d];
}

template <class Key, class Value>
inline void put(const AttributeVertexPropertyMap<Key, Value> &m, Bonded d,
                Value v) {
  if (d.get_particle()->has_attribute(m.k_)) {
    d.get_particle()->set_value(m.k_, v);
  } else {
    d.get_particle()->add_attribute(m.k_, v);
  }
}

template <class Key>
inline void put(const AttributeVertexPropertyMap<Key, Bonded> &m, Bonded d,
                Bonded v) {
  if (d.get_particle()->has_attribute(m.k_)) {
    d.get_particle()->set_value(m.k_, v.get_particle());
  } else {
    d.get_particle()->add_attribute(m.k_, v.get_particle());
  }
}

template <class Key, class Value>
struct AttributeEdgePropertyMap {
  AttributeEdgePropertyMap() {}
  Key k_;
  AttributeEdgePropertyMap(Key k) : k_(k) {}
  typedef EdgeDescriptor key_type;
  typedef Value value_type;
  struct category : public virtual boost::read_write_property_map_tag {};
  typedef value_type reference;
  value_type operator[](key_type d) const {
    Bond bd = get_bond(Bonded(d.first), Bonded(d.second));
    return typename AttributeEdgePropertyMap<Key, Value>::value_type(
        bd.get_particle()->get_value(k_));
  }
};

template <class Key, class Value>
inline typename AttributeEdgePropertyMap<Key, Value>::value_type get(
    const AttributeEdgePropertyMap<Key, Value> &m, EdgeDescriptor d) {
  return m[d];
}

template <class Key, class Value>
inline void put(const AttributeVertexPropertyMap<Key, Value> &m,
                EdgeDescriptor d, Value v) {
  Bond bd = get_bond(Bonded(d.first), Bonded(d.second));
  if (!bd.get_particle()->has_attribute(m.k_)) {
    bd.get_particle()->add_attribute(m.k_, v);
  } else {
    bd.get_particle()->set_value(m.k_, v);
  }
}

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_INTERNAL_BOND_GRAPH_FUNCTORS_H */
