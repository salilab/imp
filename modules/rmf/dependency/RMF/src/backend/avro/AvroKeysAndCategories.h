/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H
#define RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/constants.h>
#include <RMF/internal/map.h>
#include "AvroSharedData.types.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

  class AvroKeysAndCategories: public internal::SharedData {
    typedef internal::map<Category, std::string> CategoryNameMap;
    typedef internal::map<std::string, Category> NameCategoryMap;
  CategoryNameMap category_name_map_;
  NameCategoryMap name_category_map_;
  struct KeyData {
    std::string name;
    Category category;
  };
  typedef internal::map<unsigned int, KeyData> KeyDataMap;
  KeyDataMap key_data_map_;
  typedef internal::map<std::string, unsigned int> NameKeyInnerMap;
  typedef internal::map<Category, NameKeyInnerMap> NameKeyMap;
  NameKeyMap name_key_map_;

  std::vector<std::string> node_keys_;
  std::string frame_key_;

public:
  std::string get_key_name(unsigned int id) const {
    return key_data_map_.find(id)->second.name;
  }
  Category get_category(unsigned int id) const {
    return key_data_map_.find(id)->second.category;
  }
  using SharedData::get_category;

  template <class TypeTraits>
  Key<TypeTraits>
  get_key_helper(Category    category,
                 std::string name) {
    typename NameKeyInnerMap::const_iterator it
      = name_key_map_[category].find(name);
    if (it == name_key_map_[category].end()) {
      unsigned int id = key_data_map_.size();
      key_data_map_[id].name = name;
      key_data_map_[id].category = category;
      name_key_map_[category][name] = id;
      RMF_INTERNAL_CHECK(get_key_helper<TypeTraits>(category,
                                                    name)
                         == Key<TypeTraits>(id),
                         "Keys don't match");
      return Key<TypeTraits>(id);
    } else {
      int id = it->second;
      RMF_INTERNAL_CHECK(name == it->first,
                         "Odd names");
      return Key<TypeTraits>(id);
    }
  }

  const std::string &get_node_string(int node) const {
    if (node == -1) return frame_key_;
    return node_keys_[node];
  }

  template <class TypeTraits>
  const std::string &get_key_string(Key<TypeTraits> k) const {
      RMF_INTERNAL_CHECK(k.get_id() >= 0,
                       "Bad key");
    return key_data_map_.find(k.get_id())->second.name;
  }

  void clear_node_keys() {
    node_keys_.clear();
  }

  void add_node_key() {
    std::ostringstream oss;
    oss << node_keys_.size();
    node_keys_.push_back(oss.str());
  }

  std::string get_category_name(Category kc) const {
    return category_name_map_.find(kc)->second;
  }

  Categories get_categories() const {
    Categories ret;
    for (CategoryNameMap::const_iterator
         it = category_name_map_.begin(); it != category_name_map_.end();
         ++it) {
      ret.push_back(it->first);
    }
    return ret;
  }
  Category get_category(std::string name) {
    NameCategoryMap::iterator it = name_category_map_.find(name);
    if (it == name_category_map_.end()) {
      unsigned int id = category_name_map_.size();
      Category ret(id);
      name_category_map_[name] = ret;
      category_name_map_[ret] = name;
      return ret;
    } else {
      return it->second;
    }
  }


  AvroKeysAndCategories(std::string path): SharedData(path) {
  }

};

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H */
