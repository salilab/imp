/**
 *  \file AttributeTable.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ATTRIBUTE_TABLE_H
#define __IMP_ATTRIBUTE_TABLE_H

#include "../base_types.h"
#include "../utility.h"
#include "../log.h"

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/scoped_array.hpp>

#include <vector>

namespace IMP
{


namespace internal
{

/** \internal 
    If memory becomes a problem then either use a char table to look up
    actual entries or use perfect hashing. 
    http://burtleburtle.net/bob/hash/perfect.html
 */
template <class T, class VT>
class AttributeTable
{
  typedef AttributeTable<T, VT> This;
  struct Bin
  {
    bool first;
    VT second;
    Bin(): first(false){}
  };
  typedef boost::scoped_array<Bin> Map; 
  Map map_;
  unsigned int size_;

  void copy_from(unsigned int len, const Map &o) {
    IMP_assert(map_, "Internal error in attribute table");
    IMP_assert(size_ >= len, "Table too small");
    //std::cout << "Copy from " << o << " to " << map_ << std::endl;
    for (unsigned int i=0; i< len; ++i) {
      map_[i]= o[i];
    }
  }

  void realloc(unsigned int olen, const Map &o, unsigned int nlen) {
    //std::cout << "Realloc from " << size_ << " " << map_ << " ";
    if (nlen==0) {
      size_=0;
      map_.reset();
    } else {
      size_=std::max(nlen, 6U);
      map_.reset(new Bin[size_]);
      copy_from(olen, o);
    }
    //std::cout << " to " << size_ << " " << map_ << std::endl;
  }

public:
  typedef VT Value;
  typedef KeyBase<T> Key;
  AttributeTable(): size_(0){}
  AttributeTable(const This &o): size_(0) {
    //std::cout << "Copy constructor called from " << o.map_ << std::endl;
    realloc(o.size_, o.map_, o.size_);
  }
  ~AttributeTable() {
    //std::cout << "Deleting " << map_ << std::endl; 
  }
  This &operator=(const This &o) {
    //std::cout << "Operator= called from " << o.map_ << std::endl;
    if (&o == this) {
      //std::cout << "Self assignment" << std::endl;
      return *this;
    }
    realloc(o.size_, o.map_, o.size_);
    return *this;
  }
  const Value get_value(Key k) const {
    IMP_assert(contains(k),
              "Attribute \"" << k.get_string()
              << "\" not found in table.");
    return map_[k.get_index()].second;
  }


  Value& get_value(Key k) {
    IMP_assert(contains(k),
              "Attribute \"" << k.get_string()
              << "\" not found in table.");
    return map_[k.get_index()].second;
  }


  void insert(Key k, Value v);


  bool contains(Key k) const {
    IMP_assert(k != Key(), "Can't search for default key");
    return k.get_index() < size_
           && map_[k.get_index()].first;
  }


  void show(std::ostream &out, const char *prefix="") const;


  std::vector<Key> get_keys() const;

  class IsAttribute
  {
    const This *map_;
  public:
    IsAttribute(): map_(NULL){}
    IsAttribute(const This *map): map_(map) {}
    bool operator()(Key k) const {
      return map_->contains(k);
    }
  };

  typedef boost::counting_iterator<Key, boost::random_access_traversal_tag,
                                   std::size_t> KeyIterator;
  typedef boost::filter_iterator<IsAttribute, KeyIterator> AttributeKeyIterator;

  AttributeKeyIterator attribute_keys_begin() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(0U)),
                                KeyIterator(Key(size_)));
  }
  AttributeKeyIterator attribute_keys_end() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(size_)),
                                KeyIterator(Key(size_)));
  }


  unsigned int get_heap_memory_usage() const {
    return size_*sizeof(Bin);
  }

};

IMP_OUTPUT_OPERATOR_2(AttributeTable)




template <class T, class VT>
inline void AttributeTable<T, VT>::insert(Key k, Value v)
{
  /*std::cout << "Insert " << k << " in v of size " 
    << size_ << " " << map_ << " " << k.get_index() << std::endl;*/
  IMP_assert(k != Key(),
            "Can't insert default key");
  if (size_ <= k.get_index()) {
    boost::scoped_array<Bin> old;
    swap(old, map_);
    realloc(size_, old, k.get_index()+1);
  }
  IMP_assert(!map_[k.get_index()].first,
             "Trying to add attribute \"" << k.get_string()
             << "\" twice");
  map_[k.get_index()].second= v;
  map_[k.get_index()].first= true;
  IMP_assert(contains(k), "Something is broken");
}


  template <class T, class VT>
  inline void AttributeTable<T, VT>::show(std::ostream &out,
                                          const char *prefix) const
{
  for (unsigned int i=0; i< size_; ++i) {
    if (map_[i].first) {
      out << prefix;
      out << Key(i).get_string() << ": ";
      out << map_[i].second;
      out << std::endl;
    }
  }
}


template <class T, class VT>
inline std::vector<typename AttributeTable<T, VT>::Key> 
  AttributeTable<T, VT>::get_keys() const
{
  std::vector<Key> ret(attribute_keys_begin(), attribute_keys_end());
  return ret;
} 


} // namespace internal

} // namespace IMP

#endif  /* __IMP_ATTRIBUTE_TABLE_H */
