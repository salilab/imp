/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_KEYS_H
#define RMF_INTERNAL_SHARED_DATA_KEYS_H

#include "RMF/config.h"
#include "RMF/keys.h"
#include "RMF/types.h"
#include "small_set_map.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class Traits>
struct Keys {
  RMF_SMALL_UNORDERED_MAP<ID<Traits>, Category> key_categories;
  RMF_SMALL_UNORDERED_MAP<ID<Traits>, std::string> key_names;

  typedef RMF_LARGE_UNORDERED_MAP<std::string, ID<Traits> > KeyInfo;
  typedef RMF_LARGE_UNORDERED_MAP<Category, KeyInfo> CategoryKeys;
  CategoryKeys category_keys;
  // Mac OS 10.8 and earlier clang needs this for some reason
  Keys() {}
  Keys(const Keys<Traits> &o) { operator=(o); }
  Keys<Traits> &operator=(const Keys<Traits> &o) {
    category_keys = o.category_keys;
    key_categories = o.key_categories;
    key_names = o.key_names;
    return *this;
  }
};

template <class Traits>
class SharedDataKeys {
  Keys<Traits> data_;

 public:
  SharedDataKeys() {}

  Category get_category(ID<Traits> k) const {
    return data_.key_categories.find(k)->second;
  }

  ID<Traits> get_key(Category cat, std::string name, Traits) {
    typename Keys<Traits>::KeyInfo::iterator it =
        data_.category_keys[cat].find(name);
    if (it == data_.category_keys[cat].end()) {
      ID<Traits> k(data_.key_names.size());
      ensure_key(cat, k, name, Traits());
      return k;
    } else {
      return ID<Traits>(it->second);
    }
  }

  void ensure_key(Category cat, ID<Traits> key, std::string name, Traits) {
    if (data_.category_keys[cat].find(name) == data_.category_keys[cat].end()) {
      data_.category_keys[cat][name] = key;
      data_.key_names[key] = name;
      data_.key_categories[key] = cat;
    } else {
      RMF_INTERNAL_CHECK(data_.category_keys[cat].find(name)->second == key,
                         "Keys don't match");
    }
  }

  std::string get_name(ID<Traits> k) const {
    return data_.key_names.find(k)->second;
  }

  std::vector<ID<Traits> > get_keys(Category cat, Traits) const {
    if (data_.category_keys.find(cat) == data_.category_keys.end()) {
      return std::vector<ID<Traits> >();
    }
    std::vector<ID<Traits> > ret;
    ret.reserve(data_.category_keys.find(cat)->second.size());
    RMF_FOREACH(typename Keys<Traits>::KeyInfo::value_type it,
                data_.category_keys.find(cat)->second) {
      ret.push_back(it.second);
    }
    return ret;
  }
  std::vector<ID<Traits> > get_keys(Traits) const {
    std::vector<ID<Traits> > ret;
    typedef std::pair<ID<Traits>, Category> KP;
    RMF_FOREACH(KP kp, data_.key_categories) { ret.push_back(kp.first); }
    return ret;
  }

  Keys<Traits> &access_key_data(Traits) { return data_; }
  const Keys<Traits> &get_key_data(Traits) const { return data_; }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_KEYS_H */
