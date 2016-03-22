/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H
#define RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H

#include "RMF/config.h"
#include "backend/BackwardsIOBase.h"
#include "RMF/types.h"
#include "RMF/ID.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/constants.h"
#include "RMF/internal/large_set_map.h"
#include "AvroSharedData.types.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

class AvroKeysAndCategories : public backends::BackwardsIOBase {
  typedef RMF_LARGE_UNORDERED_MAP<Category, std::string> CategoryNameMap;
  typedef RMF_LARGE_UNORDERED_MAP<std::string, Category> NameCategoryMap;
  CategoryNameMap category_name_map_;
  NameCategoryMap name_category_map_;
  struct KeyData {
    std::string name;
    Category category;
  };
  typedef RMF_LARGE_UNORDERED_MAP<unsigned int, KeyData> KeyDataMap;
  KeyDataMap key_data_map_;
  typedef RMF_LARGE_UNORDERED_MAP<std::string, unsigned int> NameKeyInnerMap;
  typedef RMF_LARGE_UNORDERED_MAP<Category, NameKeyInnerMap> NameKeyMap;
  NameKeyMap name_key_map_;

  std::vector<std::string> node_keys_;
  std::string frame_key_;
  std::string get_key_name(unsigned int id) const {
    return key_data_map_.find(id)->second.name;
  }
  Category get_category_impl(unsigned int id) const {
    return key_data_map_.find(id)->second.category;
  }

 public:
  template <class Traits>
  std::string get_name(ID<Traits> k) const {
    return get_key_name(k.get_index());
  }
  template <class Traits>
  Category get_category(ID<Traits> k) const {
    return get_category_impl(k.get_index());
  }
  template <class TypeTraits>
  ID<TypeTraits> get_key(Category category, std::string name, TypeTraits) {
    typename NameKeyInnerMap::const_iterator it =
        name_key_map_[category].find(name);
    if (it == name_key_map_[category].end()) {
      unsigned int id = key_data_map_.size();
      key_data_map_[id].name = name;
      key_data_map_[id].category = category;
      name_key_map_[category][name] = id;
      RMF_INTERNAL_CHECK(get_key<TypeTraits>(category, name, TypeTraits()) ==
                             ID<TypeTraits>(id),
                         "Keys don't match");
      return ID<TypeTraits>(id);
    } else {
      int id = it->second;
      RMF_INTERNAL_CHECK(name == it->first, "Odd names");
      return ID<TypeTraits>(id);
    }
  }
  template <class TypeTraits>
  ID<TypeTraits> get_key(Category category, std::string name,
                         TypeTraits) const {
    typename NameKeyMap::const_iterator it0 = name_key_map_.find(category);
    RMF_INTERNAL_CHECK(it0 != name_key_map_.end(),
                       "Category is not found, but should be");
    typename NameKeyInnerMap::const_iterator it = it0->second.find(name);
    RMF_INTERNAL_CHECK(it != it0->second.end(),
                       "Key is not found, but should be");
    int id = it->second;
    RMF_INTERNAL_CHECK(name == it->first, "Odd names");
    return ID<TypeTraits>(id);
  }
  const std::string& get_node_string(NodeID node) const {
    if (node == NodeID()) return frame_key_;
    return node_keys_[node.get_index()];
  }

  template <class TypeTraits>
  const std::string& get_key_string(ID<TypeTraits> k) const {
    RMF_INTERNAL_CHECK(k.get_index() >= 0, "Bad key");
    return key_data_map_.find(k.get_index())->second.name;
  }

  void clear_node_keys() { node_keys_.clear(); }

  void add_node_key() {
    std::ostringstream oss;
    oss << node_keys_.size();
    node_keys_.push_back(oss.str());
  }

  std::string get_name(Category kc) const {
    return category_name_map_.find(kc)->second;
  }

  Categories get_categories() const {
    Categories ret;
    for (CategoryNameMap::const_iterator it = category_name_map_.begin();
         it != category_name_map_.end(); ++it) {
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

  AvroKeysAndCategories(std::string path) : backends::BackwardsIOBase(path) {}
};

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_AVRO_KEYS_AND_CATEGORIES_H */
