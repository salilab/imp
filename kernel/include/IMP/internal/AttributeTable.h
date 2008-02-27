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

#include <vector>

namespace IMP
{


namespace internal
{

/** \internal */
template <class T, class VT>
class AttributeTable
{
  struct Bin
  {
    bool first;
    VT second;
    Bin(): first(false){}
  };
  typedef AttributeTable<T, VT> This;
  typedef std::vector<Bin > Map;
  Map map_;
public:
  typedef VT Value;
  typedef Key<T> Key;
  AttributeTable() {} 
  const Value get_value(Key k) const {
    IMP_check(contains(k),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException((std::string("Invalid attribute \"")
                              + k.get_string() + "\" requested").c_str()));
    return map_[k.get_index()].second;
  }

  Value& get_value(Key k) {
    IMP_check(contains(k),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException((std::string("Invalid attribute \"")
                              + k.get_string() + "\" requested").c_str()));
    return map_[k.get_index()].second;
  }
  void insert(Key k, Value v);
  bool contains(Key k) const {
    IMP_check(k != Key(), "Can't search for default key",
              IndexException("Bad index"));
    return k.get_index() < map_.size()
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
                                KeyIterator(Key(map_.size())));
  }
  AttributeKeyIterator attribute_keys_end() const {
    return AttributeKeyIterator(IsAttribute(this),
                                KeyIterator(Key(map_.size())),
                                KeyIterator(Key(map_.size())));
  }


};

IMP_OUTPUT_OPERATOR_2(AttributeTable)




template <class T, class VT>
inline void AttributeTable<T, VT>::insert(Key k, Value v)
{
  IMP_check(k != Key(),
            "Can't insert default key",
            IndexException("bad index"));
  if (map_.size() <= k.get_index()) {
    map_.resize(k.get_index()+1);
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
  for (unsigned int i=0; i< map_.size(); ++i) {
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

inline void show_attributes(std::ostream &out)
{
  if (attribute_key_data.size() < attribute_table_index(Float())) {
    out << "Float attributes are ";
    for (unsigned int i=0;
         i< attribute_key_data[attribute_table_index(Float())].rmap.size();
         ++i) {
      out << "\""
      << attribute_key_data[attribute_table_index(Float())].rmap[i]
      << "\" ";
    }
    out << std::endl;
  }
  if (attribute_key_data.size() < attribute_table_index(Int())) {
    out << "Int attributes are ";
    for (unsigned int i=0;
         i< attribute_key_data[attribute_table_index(Int())].rmap.size();
         ++i) {
      out << "\""
      << attribute_key_data[attribute_table_index(Int())].rmap[i]
      << "\" ";
    }
  }
  if (attribute_key_data.size() < attribute_table_index(String())) {
    out << "String attributes are ";
    for (unsigned int i=0;
         i< attribute_key_data[attribute_table_index(String())].rmap.size();
         ++i) {
      out << "\""
      << attribute_key_data[attribute_table_index(String())].rmap[i]
      << "\" ";
    }
    out << std::endl;
  }

} 


} // namespace internal

} // namespace IMP

#endif  /* __IMP_ATTRIBUTE_TABLE_H */
