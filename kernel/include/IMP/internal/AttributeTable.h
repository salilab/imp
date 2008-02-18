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
#include "../ModelData.h"

#include <map>
#include <vector>

namespace IMP
{

class ModelData;

namespace internal
{

/** \internal */
template <class T>
class AttributeTable
{
  std::vector<Index<T> > map_;
public:
  typedef Index<T> Value;
  typedef Key<T> Key;
  AttributeTable() {} 
  const Value get_value(Key k) const {
    IMP_check(contains(k),
              "Attribute \"" << k.get_string()
              << "\" not found in table.",
              IndexException((std::string("Invalid attribute \"")
                              + k.get_string() + "\" requested").c_str()));
    return map_[k.get_index()];
  }
  void insert(Key k, Value v);
  bool contains(Key k) const {
    IMP_check(k != Key(), "Can't search for default key",
              IndexException("Bad index"));
    return k.get_index() < map_.size()
           && map_[k.get_index()] != Value();
  }
  void show(std::ostream &out, const char *prefix="",
            ModelData* md=NULL) const;
  std::vector<Key> get_keys() const;
};

IMP_OUTPUT_OPERATOR_1(AttributeTable)




template <class T>
inline void AttributeTable<T>::insert(Key k, Value v)
{
  IMP_check(k != Key(),
            "Can't insert default key",
            IndexException("bad index"));
  IMP_assert(v != Value(),
             "Can't add attribute with no index");
  if (map_.size() <= k.get_index()) {
    map_.resize(k.get_index()+1);
  }
  IMP_assert(map_[k.get_index()]== Value(),
             "Trying to add attribute \"" << k.get_string()
             << "\" twice");
  map_[k.get_index()]= v;
  IMP_assert(contains(k), "Something is broken");
}


template <class T>
inline void AttributeTable<T>::show(std::ostream &out,
                                    const char *prefix,
                                    ModelData *md) const
{
  for (unsigned int i=0; i< map_.size(); ++i) {
    if (map_[i] != Value()) {
      out << prefix;
      if (md != NULL) {
        out << md->get_value(map_[i]);
      }
      out << std::endl;
    }
  }
}


template <class T>
inline std::vector<typename AttributeTable<T>::Key> 
AttributeTable<T>::get_keys() const
{
  std::vector<Key> ret;
   for (unsigned int i=0; i< map_.size(); ++i) {
     if (map_[i] != Value()) {
       ret.push_back(Key(i));
     }
   }
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
