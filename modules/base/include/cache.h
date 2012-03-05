/**
 *  \file IMP/base/cache.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CACHE_H
#define IMPBASE_CACHE_H

#include "base_config.h"
#include "internal/cache.h"
#include "exception.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <functional>

#ifdef _MSC_VER
#include <cstddef> // for offsetof
#endif

IMPBASE_BEGIN_NAMESPACE



/** This is a helper class for writing memoizers: things that
    store the results of a computation to look up later. The result
    type must support
    - \c operator=
    - \c operator==
    - default construction
*/
template <class Generator,
          class Checker=std::equal_to<typename Generator::result_type> >
class Memoizer {
  Generator gen_;
  Checker checker_;
  mutable bool has_result_;
  mutable typename Generator::result_type result_;
  mutable int num_misses_;
  mutable int num_stats_;
public:
  typedef typename Generator::result_type Value;
  typedef typename Generator::argument_type Key;
  Memoizer(const Generator &gen,
           const Checker &checker=Checker()): gen_(gen),
                                              checker_(checker),
                                              has_result_(false),
                                              num_misses_(0),
                                              num_stats_(0){}
  Generator &access_generator() {return gen_;}
  const Generator &get_generator() const {return gen_;}
  void reset() {
    has_result_=false;
    result_= Value();
  }
  const Value &get() const {
    if (!has_result_) {
      result_= gen_();
      has_result_=true;
      ++num_misses_;
    }
    ++num_stats_;
    IMP_INTERNAL_CHECK(checker_(result_,
                               gen_()),
                       "Wrong result returned from generator");
     return result_;
  }
  //! Update the stored result manually
  void set(const Value &v) const {
    IMP_INTERNAL_CHECK(checker_(v, gen_()),
                       "Wrong result passed on set");
    has_result_=true;
    result_=v;
  }
  double get_hit_rate() const {
    return 1.0-static_cast<double>(num_misses_)/num_stats_;
  }
};

#if 0
/** Implement a cache on sparse pairs of values. The cache
    is infinite (or at least n^2).
*/
template <class Generator, class Checker>
class SparseSymmetricPairCache {
public:
  typedef typename Generator::first_argument_type Key;
  typedef typename Generator::result_type Value;
private:
  Generator gen_;
  Checker checker_;
  unsigned long count_;
  mutable int num_stats_;
  mutable int num_misses_;

  struct Data {
    Key k0, k1;
    Value v;
    Data(Key ik0, Key ik1, const Value &iv) {
      k0=ik0;
      k1=ik1;
      v=iv;
    }
  };
  typedef boost::multi_index::member<Data,
                                     Key,
                                     &KVP::k0> Key0Member;
  typedef boost::multi_index::member<Data,
                                     Key,
                                     &KVP::k1> Key1Member;
  typedef boost::multi_index::hashed_non_unique<Key0Member> Hash0Index;
  typedef boost::multi_index::hashed_non_unique<Key1Member> Hash1Index;
  typedef boost::multi_index_container<Data,
                             boost::multi_index::indexed_by<Hash0Index,
                                                         Hash1Index > > Map;
  mutable Map map_;
  typedef typename boost::multi_index::template nth_index<Map, 0>
  ::type::const_iterator LookupIterator;
  typedef typename boost::multi_index::template nth_index<Map, 1>
  ::type::const_iterator OrderIterator;

  template <class Index, class It>
  static void erase(Index &index, std::pair<It,It> range) {
    index.erase(range.first, range.second);
  }

  void remove(Key k) const {
    erase(map_.get<0>(), map_.get<0>().equal_range(k));
    erase(map_.get<1>(), map_.get<1>().equal_range(k));
  }
public:
  SparsePairCache(const Generator &gen,
                  const Checker &check): gen_(gen), checker_(check){}
  const Value &get(const Key &a,
                   const Key &b) const {

  }
  //! Get all values involving a
  vector<std::pair<Key, Value> > get(const Key &a) const;
  void reset();
  //! Reset all pairs involving a
  void reset(const Key &a);
  double get_hit_rate() const;
};

#endif

/** Implement a simple least recently used cache. As with
    the Memoizer, it is parameterized by a generator that is
    used to generate values if they are not in the cache.

    The Generator should have a method:
    - Generator::operator()(Key, Cache);
*/
template <class Generator,
          class Checker=std::equal_to<typename Generator::result_type> >
class LRUCache {
public:
  typedef typename Generator::result_type Value;
  typedef typename Generator::argument_type Key;
private:
  Generator gen_;
  Checker checker_;
  unsigned long count_;
  unsigned int max_size_;
  mutable int num_stats_;
  mutable int num_misses_;
  struct KVP {
    Key key;
    Value value;
    KVP(const Key &k, const Value &v): key(k), value(v){}
  };
  typedef boost::multi_index::member<KVP,
                                     Key,
                                     &KVP::key > KeyMember;
  typedef boost::multi_index::hashed_unique<KeyMember> HashIndex;
  typedef boost::multi_index::sequenced< > Sequenced;
  typedef boost::multi_index_container<KVP,
                                       boost::multi_index::indexed_by<HashIndex,
                                                         Sequenced > > Map;
  mutable Map map_;
  typedef typename boost::multi_index::template nth_index<Map, 0>
  ::type::const_iterator LookupIterator;
  typedef typename boost::multi_index::template nth_index<Map, 1>
  ::type::const_iterator OrderIterator;
  LookupIterator add_value(const Key &k) const {
    Value v= gen_(k, this);
    OrderIterator it= map_.template get<1>().push_front(KVP(k, v)).first;
    while (map_.size() > max_size_) {
      map_.template get<1>().pop_back();
    }
    return map_.template project<0>(it);
  }
public:
  LRUCache(const Generator &gen, unsigned int size,
           const Checker checker=Checker()): gen_(gen),
                                             checker_(checker),
                                             max_size_(size),
                                             num_stats_(0),
                                             num_misses_(0){}
  const Value &get(const Key &k) const {
    LookupIterator it=map_.template get<0>().find(k);
    if (it == map_.template get<0>().end()) {
      it=add_value(k);
      ++num_misses_;
    }
    ++num_stats_;
    map_.template get<1>().relocate(map_.template project<1>(it),
                                    map_.template get<1>().begin());
    IMP_INTERNAL_CHECK(checker_(it->value,
                                gen_(k, this)),
                       "Results don't match.");
    return it->value;

  }
  double get_hit_rate() const {
    return 1.0-static_cast<double>(num_misses_)/num_stats_;
  }
  compatibility::vector<Key> get_keys() const {
    compatibility::vector<Key> ret;
    for (OrderIterator it= map_.template get<1>().begin();
         it != map_.template get<1>().end(); ++it) {
      ret.push_back(it->key);
    }
    return ret;
  }
};


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CACHE_H */
