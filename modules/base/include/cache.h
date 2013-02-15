/**
 *  \file IMP/base/cache.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CACHE_H
#define IMPBASE_CACHE_H

#include <IMP/base/base_config.h>
#include "internal/cache.h"
#include "check_macros.h"
#include "log_macros.h"
#include "Vector.h"
#include <IMP/base/hash.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/global_fun.hpp>
#include "Array.h"
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


/** Implement a cache on sparse pairs of values. The cache
    is infinite (or at least n^2).
*/
template <class Generator, class Checker>
class SparseSymmetricPairMemoizer {
public:
  typedef typename Generator::argument_type::value_type Key;
  typedef typename Generator::result_type::value_type Entry;
  typedef SparseSymmetricPairMemoizer<Generator, Checker> This;
private:
  Generator gen_;
  Checker checker_;

  static Key get_0( Entry e) {return e[0];}
  static Key get_1( Entry e) {return e[1];}

  // The This:: is needed to make certain gcc versions (4.7) happier
  typedef boost::multi_index::global_fun<Entry,
                                         Key,
                                         &This::get_0 > P0Member;
  typedef boost::multi_index::global_fun<Entry,
                                         Key,
                                         &This::get_1 > P1Member;
  typedef boost::multi_index::hashed_non_unique<P0Member> Hash0Index;
  typedef boost::multi_index::hashed_non_unique<P1Member> Hash1Index;
  typedef boost::multi_index::indexed_by<Hash0Index,
                                         Hash1Index > IndexBy;
  typedef boost::multi_index_container<Entry,
                                       IndexBy> Cache;
  typedef typename boost::multi_index::nth_index<Cache, 0>
  ::type::const_iterator Hash0Iterator;
  typedef typename boost::multi_index::nth_index<Cache, 1>
  ::type::const_iterator Hash1Iterator;
  Cache cache_;
  Vector<Key> cleared_, domain_;

  struct EntryEqual {
    Array<2, Key> v;
    EntryEqual(Key t0, Key t1):
      v(t0, t1){}
    template <class O>
    bool operator()(const O &o) const {
      return v[0]==o[0] && v[1] == o[1];
    }
  };

  Hash0Iterator get(Key t0, Key t1) const {
    Hash0Iterator b,e;
    boost::tie(b,e)=cache_.template get<0>().equal_range(t0);
    /*IMP_LOG_VERBOSE( "Found first matches "
      << Vector<Entry>(b,e) << " for " << t0 << std::endl);*/
    Hash0Iterator f= std::find_if(b,e, EntryEqual(t0, t1));
    // otherwise it returns something not equal end()
    if (f==e) return cache_.template get<0>().end();
    else return f;
  }

  void check_it() const {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    Vector<Entry> cur(cache_.begin(), cache_.end());
    IMP_INTERNAL_CHECK(checker_(cur),
                       "Cached and newly computed don't match: "
                       << cur << " vs " << gen_(domain_, this)
                       << " and cleared is " << cleared_);
    for (Hash0Iterator c= cache_.template get<0>().begin();
         c != cache_.template get<0>().end(); ++c) {
      IMP_INTERNAL_CHECK(get(c->operator[](1), c->operator[](0))
                         == cache_.template get<0>().end(),
                         "Both an entry and its flip are in the table: "
                         << *c << ": " << cur);
    }
#endif
  }
  void fill_it() {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< cleared_.size(); ++i) {
        {
          Hash0Iterator b,e;
          boost::tie(b,e)=cache_.template get<0>().equal_range(cleared_[i]);
          IMP_INTERNAL_CHECK(b==e, "Cleared entry " << cleared_[i]
                             << " was not cleared.");
        }
        {
          Hash1Iterator b,e;
          boost::tie(b,e)=cache_.template get<1>().equal_range(cleared_[i]);
          IMP_INTERNAL_CHECK(b==e, "Cleared entry " << cleared_[i]
                             << " was not cleared.");
        }
      }
    }
    IMP_LOG_VERBOSE( "Filling from " << cleared_ << std::endl);
    Vector<Entry> nv= gen_(cleared_,  *this);
    IMP_LOG_VERBOSE( "Inserting " << nv << " into pair memoizer" << std::endl);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (unsigned int i=0; i< nv.size(); ++i) {
        IMP_INTERNAL_CHECK(std::find_if(nv.begin(), nv.end(),
                                     EntryEqual(nv[i][1],
                                                nv[i][0]))
                           == nv.end(),
                           "An entry and its flip are already in list: "
                           << nv);

      }
    }
    cache_.insert(nv.begin(), nv.end());
    check_it();
    IMP_LOG_VERBOSE( "To get "
            << typename Generator::result_type(cache_.begin(),
                                               cache_.end())
            << std::endl);
    cleared_.clear();
  }
  template <class F, class It>
  F do_apply( It b, It e, F f) const {
    for (It c=b; c!= e; ++c) {
      f(*c);
    }
    return f;
  }
public:
  SparseSymmetricPairMemoizer(const Vector<Key> &domain,
                              const Generator &gen= Generator(),
                              const Checker &check= Checker()):
    gen_(gen),
    checker_(check),
    cleared_(domain),
    domain_(domain){
    IMP_LOG_TERSE( "Domain for memoizer is " << domain << std::endl);
  }
  template <class F>
  F apply(F f) {
    IMP_FUNCTION_LOG;
    if (!cleared_.empty()) fill_it();
    check_it();
    return do_apply(cache_.begin(), cache_.end(), f);
  }
  /** Apply a function to the current (unfilled) state of the memoizer.*/
  template <class F>
  F apply_to_current_contents(F f) {
    IMP_FUNCTION_LOG;
    return do_apply(cache_.begin(), cache_.end(), f);
  }
  //! Clear all entries involve the Key
  /** The removed entries are returned */
  void remove(const Key &a) {
    if (std::find(cleared_.begin(), cleared_.end(), a) != cleared_.end()) {
      return;
    }
    Vector<Entry> ret;
    cleared_.push_back(a);
    {
      Hash0Iterator b,e;
      boost::tie(b,e)=cache_.template get<0>().equal_range(a);
      cache_.template get<0>().erase(b,e);
    }
    {
      Hash1Iterator b,e;
      boost::tie(b,e)=cache_.template get<1>().equal_range(a);
      cache_.template get<1>().erase(b,e);
    }
  }
  void insert(const Entry &e) {
    cache_.insert(e);
  }
  void clear() {
    cache_.clear();
    cleared_=domain_;
  }
  const Generator &get_generator() const {return gen_;}
  Generator &access_generator() const {return gen_;}
};


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
  Value add_value(const Key &k) const {
    Value v= gen_(k, *this);
    map_.template get<1>().push_front(KVP(k, v));
    while (map_.size() > max_size_) {
      IMP_LOG_VERBOSE( "Cache overflow" << std::endl);
      map_.template get<1>().pop_back();
    }
    return v;
  }
public:
  LRUCache(const Generator &gen, unsigned int size,
           const Checker checker=Checker()): gen_(gen),
                                             checker_(checker),
                                             max_size_(size),
                                             num_stats_(0),
                                             num_misses_(0){}
  Value get(const Key &k) const {
    LookupIterator it=map_.template get<0>().find(k);
    ++num_stats_;
    if (it == map_.template get<0>().end()) {
      IMP_LOG_VERBOSE( "Cache miss on " << k << std::endl);
      ++num_misses_;
      Value v=add_value(k);
      IMP_INTERNAL_CHECK(max_size_==0 || map_.template get<0>().find(k)
                         != map_.template get<0>().end(),
                         "Failed to insert into cache");
      return v;
    } else {
      map_.template get<1>().relocate(map_.template project<1>(it),
                                      map_.template get<1>().begin());
      // not good with floating point values
      /*IMP_INTERNAL_CHECK(checker_(it->value,
                                  gen_(k, *this)),
                         "Results don't match: " << it->value << " != "
                         << gen_(k, *this));*/
      IMP_INTERNAL_CHECK(map_.template get<0>().find(k)
                         != map_.template get<0>().end(),
                         "Gone, gone I tell you");
      return it->value;
    }

  }
  double get_hit_rate() const {
    return 1.0-static_cast<double>(num_misses_)/num_stats_;
  }
  Vector<Key> get_keys() const {
    Vector<Key> ret;
    for (OrderIterator it= map_.template get<1>().begin();
         it != map_.template get<1>().end(); ++it) {
      ret.push_back(it->key);
    }
    return ret;
  }
  typedef OrderIterator ContentIterator;
  ContentIterator contents_begin() const {
    return map_.template get<1>().begin();
  }
  ContentIterator contents_end() const {
    return map_.template get<1>().end();
  }
  void insert(Key k, Value v) {
    LookupIterator it=map_.template get<0>().find(k);
    if (it == map_.template get<0>().end()) {
      map_.template get<1>().push_front(KVP(k, v));
      while (map_.size() > max_size_) {
        map_.template get<1>().pop_back();
      }
    }
  }
  unsigned int size() const {
    return map_.size();
  }
  Generator &access_generator() {
    return gen_;
  }
  const Generator &get_generator() const {
    return gen_;
  }
};


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_CACHE_H */
