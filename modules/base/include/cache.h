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

IMPBASE_BEGIN_NAMESPACE



/** This is a helper class for writing memoizers: things that
    store the results of a computation to look up later. The result
    type must support
    - \c operator=
    - \c operator==
    - default construction
*/
template <class Generator, bool ORDERED=true>
class Memoizer {
  Generator gen_;
  mutable bool has_result_;
  mutable typename Generator::result_type result_;
  mutable int num_misses_;
  mutable int num_stats_;
public:
  typedef typename Generator::result_type Value;
  typedef typename Generator::argument_type Key;
  Memoizer(const Generator &gen): gen_(gen),
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
    IMP_INTERNAL_CHECK(internal::check_result<ORDERED>(result_),
                       "Wrong result returned from generator");
    return result_;
  }
  double get_hit_rate() const {
    return 1.0-static_cast<double>(num_misses_)/num_stats_;
  }
};

struct Order{};
struct Lookup{};


/** Implement a simple least recently used cache. As with
    the Memoizer, it is parameterized by a generator that is
    used to generate values if they are not in the cache.
*/
template <class Generator, bool ORDERED=true>
class LRUCache {
public:
  typedef typename Generator::result_type Value;
  typedef typename Generator::argument_type Key;
private:
  Generator gen_;
  unsigned long count_;
  unsigned int max_size_;
  mutable int num_stats_;
  mutable int num_misses_;
  typedef boost::multi_index::tag<Order> OrderTag;
  typedef boost::multi_index::tag<Lookup> LookupTag;
  typedef std::pair<Key, Value> KVP;
  typedef boost::multi_index::member<KVP,
                                     Key,
                                     &KVP::first> KeyMember;
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
    Value v= gen_(k);
    OrderIterator it= map_.template get<1>().push_front(KVP(k, v)).first;
    if (map_.size() > max_size_) {
      map_.template get<1>().pop_back();
    }
    return map_.template project<0>(it);
  }
public:
  LRUCache(const Generator &gen, unsigned int size): gen_(gen),
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
    IMP_INTERNAL_CHECK(internal::check_result<false>(it->second,
                                    gen_(k)),
                       "Results don't match.");
    return it->second;
  }
  double get_hit_rate() const {
    return 1.0-static_cast<double>(num_misses_)/num_stats_;
  }
  compatibility::vector<Key> get_keys() const {
    compatibility::vector<Key> ret;
    for (OrderIterator it= map_.template get<1>().begin();
         it != map_.template get<1>().end(); ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
};


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CACHE_H */
