/**
 *  \file MapHelper.h   \brief A helper for the attribute map
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSEARCH_INTERNAL_MAP_HELPERS_H
#define IMPSEARCH_INTERNAL_MAP_HELPERS_H

#include "../config.h"
#include <IMP/base_types.h>

#include <IMP/Particle.h>

#include <boost/tuple/tuple.hpp>
#include <boost/static_assert.hpp>

#include <iostream>
#include <map>

IMPSEARCH_BEGIN_INTERNAL_NAMESPACE

template <class K>
struct KeyValue {
};
template <>
struct KeyValue<FloatKey> {
  typedef Float Value;
};
template <>
struct KeyValue<IntKey> {
  typedef Int Value;
};
template <>
struct KeyValue<StringKey> {
  typedef String Value;
};
template <>
struct KeyValue<ParticleKey> {
  typedef Particle* Value;
};

template <int N, class T>
struct TupleCompare {
  static int compare(const T &a, const T &b) {
    int c=0;
    if (boost::get<N-1>(a) < boost::get<N-1>(b)) c=-1;
    else if (boost::get<N-1>(a) > boost::get<N-1>(b)) c=1;
    if (c==0) {
      return TupleCompare<N-1, T>::compare(a,b);
    } else {
      return c;
    }
  }
};

template <class T>
struct TupleCompare<0, T> {
  static int compare(const T &a, const T &b) {
    return 0;
  }
};


template <int N, class T>
struct TupleWriter {
  static void write(const T& a, std::ostream &out) {
    out << boost::get<N-1>(a) << " ";
    TupleWriter<N-1, T>::write(a, out);
  }
};

template <class T>
struct TupleWriter<0, T> {
  static void write(const T &a, std::ostream &out) {}
};


template <int N, class K>
struct ValueTuple {
};
template <class K>
struct ValueTuple<1, K> {
  typedef typename boost::tuples::element<0, K>::type Key0;
  typedef typename KeyValue<Key0>::Value Value0;
  typedef boost::tuple< Value0 > Value;
  static Value get_value(const Particle *p, const K &k) {
    Key0 k0=k.template get<0>();
    Value0 v0=p->get_value(k0);
    return Value(v0);
  }
};

template <class K>
struct ValueTuple<2, K> {
  typedef typename boost::tuples::element<0, K>::type Key0;
  typedef typename KeyValue<Key0>::Value Value0;
  typedef typename boost::tuples::element<1, K>::type Key1;
  typedef typename KeyValue<Key1>::Value Value1;
  typedef boost::tuple< Value0, Value1 > Value;
  static Value get_value(const Particle *p, const K &k) {
    Key0 k0=k.template get<0>();
    Value0 v0=p->get_value(k0);
    Key1 k1=k.template get<1>();
    Value1 v1=p->get_value(k1);
    return Value(v0, v1);
  }
};



template <class K>
struct MapTraits {
  typedef MapTraits<K> ThisTraits;
  typedef ValueTuple<boost::tuples::length<K>::value, K> VT;
  typedef typename VT::Value Value;
  static Value get_value(Particle *p, const K &k) {
    return VT::get_value(p, k);
  }
  static int compare(const Value& a, const Value &b) {
    return TupleCompare<boost::tuples::length<K>::value, Value>::compare(a,b);
  }
  static void write(const K &k, std::ostream &out) {
    TupleWriter<boost::tuples::length<K>::value, K>::write(k, out);
  }
  struct Bin {
    Particles ps_;
    Value v_;
    Bin(const Value &v): v_(v){}
    bool operator<(const Bin &o) const {
      //return ThisTraits::compare(v_, o.v_) == -1;
      return TupleCompare<boost::tuples::length<K>::value, Value>
      ::compare(v_,o.v_)==-1;
    }
    bool operator==(const Bin &o) const {
      return TupleCompare<boost::tuples::length<K>::value, Value>
      ::compare(v_,o.v_) ==0;
      //return ThisTraits::compare(v_, o.v_)==0;
    }
  };
};
IMPSEARCH_END_INTERNAL_NAMESPACE

#endif  /* IMPSEARCH_INTERNAL_MAP_HELPERS_H */
