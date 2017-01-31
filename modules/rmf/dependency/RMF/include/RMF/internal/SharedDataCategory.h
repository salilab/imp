/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_CATEGORY_H
#define RMF_INTERNAL_SHARED_DATA_CATEGORY_H

#include "RMF/config.h"
#include "RMF/ID.h"
#include "small_set_map.h"
#include "RMF/infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

struct CategoryData {
  typedef RMF_SMALL_UNORDERED_MAP<std::string, Category> From;
  From from_name;
  typedef RMF_SMALL_UNORDERED_MAP<Category, std::string> To;
  To to_name;
  // Mac OS 10.8 and earlier Clang needs these for some reason
  CategoryData() {}
  CategoryData &operator=(const CategoryData &o) {
    from_name = o.from_name;
    to_name = o.to_name;
    return *this;
  }
  CategoryData(const CategoryData &o)
      : from_name(o.from_name), to_name(o.to_name) {}
};

class SharedDataCategory {
  CategoryData data_;

 public:
  SharedDataCategory() {}
  Categories get_categories() const {
    Categories ret;
    RMF_FOREACH(const CategoryData::From::value_type & it, data_.from_name) {
      ret.push_back(it.second);
    }
    return ret;
  }
  Category get_category(std::string name) {
    CategoryData::From::const_iterator it = data_.from_name.find(name);
    if (it == data_.from_name.end()) {
      Category ret(data_.from_name.size());
      data_.from_name[name] = ret;
      data_.to_name[ret] = name;
      return ret;
    } else {
      return it->second;
    }
  }

  void ensure_category(Category category, std::string name) {
    if (data_.from_name.find(name) == data_.from_name.end()) {
      data_.from_name[name] = category;
      data_.to_name[category] = name;
    } else {
      RMF_INTERNAL_CHECK(data_.from_name.find(name)->second == category,
                         "Does not match existing category");
    }
  }

  std::string get_name(Category kc) const {
    return data_.to_name.find(kc)->second;
  }

  const CategoryData &get_category_data() const { return data_; }
  CategoryData &access_category_data() { return data_; }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_CATEGORY_H */
