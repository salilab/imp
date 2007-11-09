/**
 *  \file AttributeKey.h    \brief Keys to cache lookup of attribute strings.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_ATTRIBUTE_KEY_H
#define __IMP_ATTRIBUTE_KEY_H

#include "utility.h"
#include "log.h"
#include <map>


namespace IMP
{

namespace internal {
struct AttributeKeyData {
  std::map<std::string, int> map;
  std::vector<std::string> rmap;
};

inline unsigned int attribute_table_index(Float)
{
  return 0;
}

inline unsigned int attribute_table_index(Int)
{
  return 1;
}

inline unsigned int attribute_table_index(String)
{
  return 2;
}

extern IMPDLLEXPORT std::vector<AttributeKeyData> attribute_key_data;

template <class T>
class AttributeTable;
}

//! A base class for Attribute Keys
/** This class does internal caching of the strings to accelerate the
    name lookup. It is better to create an AttributeKey and reuse it
    rather than recreate it many times from strings.

    If you use this with a new type, you must add a new definition of
    attribute_table_index. Yes, this is an evil hack, but I couldn't
    get linking to work with static members of the template class.
 */
template <class T>
class AttributeKey
{
  friend class internal::AttributeTable<T>;
  int str_;

  static internal::AttributeKeyData& data() {
    unsigned int i= internal::attribute_table_index(T());
    if ( internal::attribute_key_data.size() <= i) {
      internal::attribute_key_data.resize(i+1);
    }
    return internal::attribute_key_data[i];
  }

  bool is_default() const {
    return str_==-1;
  }
  static const std::string &get_string(int i) {
    IMP_assert(static_cast<unsigned int>(i)
               < data().rmap.size(),
               "Corrupted "  << " AttributeKey " << i
               << " vs " << data().rmap.size());
    return data().rmap[i];
  }
public:



  typedef AttributeKey<T> This;

  //! make a default (uninitalized) key
  AttributeKey():str_(-1) {}
  //! Generate a key from the given string
  AttributeKey(const char *c) {
    std::string sc(c);
    if (data().map.find(sc) == data().map.end()) {

      int sz= data().map.size();
      data().map[sc]=sz;
      data().rmap.push_back(sc);
      str_= sz;
      IMP_assert(data().rmap.size() == data().map.size(), "Unequal map sizes")
    } else {
      str_= data().map.find(sc)->second;
    }
    //str_=c;
  };


  //! Turn a key into a pretty string
  const std::string get_string() const {
    if (is_default()) return std::string("NULL");
    return get_string(str_);
    //return str_;
  }

  IMP_COMPARISONS_1(str_)

  std::ostream &show(std::ostream &out) const {
    return out << "\"" << get_string() << "\"";
  }

  unsigned int get_index() const {
    IMP_assert(!is_default(),
               "Cannot get index on defaultly constructed AttributeKey");
    return str_;
  }
};

IMP_OUTPUT_OPERATOR_1(AttributeKey)


namespace internal {

template <class T>
class AttributeTable
{
  std::vector<Index<T> > map_;
public:
  typedef Index<T> Value;
  typedef AttributeKey<T> Key;
  AttributeTable() {}
  const Value operator[](Key k) const;
  void insert(Key k, Value v);
  bool contains(Key k) const;
  std::ostream &show(std::ostream &out, const char *prefix="") const;
};

IMP_OUTPUT_OPERATOR_1(AttributeTable)

}

}

#endif  /* __IMP_ATTRIBUTE_KEY_H */
