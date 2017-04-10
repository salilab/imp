/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF_5SHARED_DATA_H
#define RMF_INTERNAL_HDF_5SHARED_DATA_H

#include <H5public.h>
#include <boost/array.hpp>
#include <boost/make_shared.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <hdf5.h>
#include <stddef.h>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "HDF5DataSetCache1D.h"
#include "HDF5DataSetCache2D.h"
#include "HDF5DataSetCache3D.h"
#include "HDF5DataSetCacheD.h"
#include "RMF/HDF5/DataSetIndexD.h"
#include "RMF/HDF5/File.h"
#include "RMF/HDF5/Group.h"
#include "RMF/ID.h"
#include "RMF/compiler_macros.h"
#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/large_set_map.h"
#include "RMF/log.h"
#include "RMF/traits.h"
#include "RMF/types.h"
#include "backend/BackwardsIOBase.h"
#include "backend/backward_types.h"
#include "names.h"

namespace boost {
template <class T>
struct nullable;
}  // namespace boost

RMF_ENABLE_WARNINGS

namespace RMF {

namespace hdf5_backend {
using namespace RMF::backward_types;

#define RMF_HDF5_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                                  PassValues, ReturnValues)               \
  DataDataSetCache2D<Ucname##Traits> lcname##_data_sets_;                 \
  DataDataSetCache3D<Ucname##Traits> per_frame_##lcname##_data_sets_;     \
  HDF5DataSetCacheD<Ucname##Traits, 2>& get_data_set_i(                   \
      Ucname##Traits, unsigned int kc, int arity) const {                 \
    return lcname##_data_sets_.get(file_, kc, get_category_name_impl(kc), \
                                   arity);                                \
  }                                                                       \
  HDF5DataSetCacheD<Ucname##Traits, 3>& get_per_frame_data_set_i(         \
      Ucname##Traits, unsigned int kc, int arity) const {                 \
    return per_frame_##lcname##_data_sets_.get(                           \
        file_, kc, get_category_name_impl(kc), arity);                    \
  }

class HDF5SharedData : public backends::BackwardsIOBase {
  // indexed first by per frame, then by
  // TypeInfo::get_index() then by ID
  // then by key.get_index()
  mutable HDF5::Group file_;
  bool read_only_;
  HDF5DataSetCacheD<StringTraits, 1> node_names_;
  HDF5DataSetCacheD<StringTraits, 1> frame_names_;
  HDF5DataSetCacheD<StringTraits, 1> category_names_;
  HDF5DataSetCacheD<IndexTraits, 2> node_data_;
  Ints free_ids_;
  unsigned int frames_hint_;

  struct CategoryData {
    int index;
    std::string name;
  };

  typedef RMF_LARGE_UNORDERED_MAP<Category, CategoryData> CategoryDataMap;
  CategoryDataMap category_data_map_;
  typedef RMF_LARGE_UNORDERED_MAP<std::string, Category> NameCategoryMap;
  NameCategoryMap name_category_map_;

  struct KeyData {
    int static_index, per_frame_index;
    std::string name;
    Category category;
    int type_index;
  };

  // Using unordered_map here causes segfaults on Ubuntu 12.04; possibly
  // a boost or HDF5 bug
  typedef RMF_LARGE_ORDERED_MAP<unsigned int, KeyData> KeyDataMap;

  KeyDataMap key_data_map_;
  typedef RMF_LARGE_UNORDERED_MAP<std::string, unsigned int> NameKeyInnerMap;
  typedef RMF_LARGE_UNORDERED_MAP<Category, NameKeyInnerMap> NameKeyMap;
  NameKeyMap name_key_map_;

  Category link_category_;
  ID<NodeIDTraits> link_key_;

  // caches
  typedef std::vector<std::vector<int> > IndexCache;
  mutable IndexCache index_cache_;

  template <class TypeTraits>
  class DataDataSetCache3D {
    typedef HDF5DataSetCacheD<TypeTraits, 3> DS;
    mutable boost::ptr_vector<boost::nullable<DS> > cache_;
    FrameID frame_;

   public:
    DataDataSetCache3D() : frame_(0) {}
    HDF5DataSetCacheD<TypeTraits, 3>& get(HDF5::Group file,
                                          unsigned int category_index,
                                          std::string kcname, int arity) const {
      bool found = true;
      if (cache_.size() <= category_index) {
        found = false;
      } else if (cache_.is_null(category_index)) {
        found = false;
      }
      if (!found) {
        std::string nm = get_data_data_set_name(
            kcname, arity, TypeTraits::HDF5Traits::get_name(), true);
        cache_.resize(
            std::max(cache_.size(), static_cast<size_t>(category_index + 1)),
            NULL);
        cache_.replace(category_index, new DS());
        cache_[category_index].set_current_frame(frame_.get_index());
        cache_[category_index].set(file, nm);
      }
      return cache_[category_index];
    }
    void set_current_frame(FrameID f) {
      frame_ = f;
      for (unsigned int i = 0; i < cache_.size(); ++i) {
        if (!cache_.is_null(i)) {
          cache_[i].set_current_frame(f.get_index());
        }
      }
    }
  };
  template <class TypeTraits>
  class DataDataSetCache2D {
    typedef HDF5DataSetCacheD<TypeTraits, 2> DS;
    mutable boost::ptr_vector<boost::nullable<DS> > cache_;

   public:
    HDF5DataSetCacheD<TypeTraits, 2>& get(HDF5::Group file,
                                          unsigned int category_index,
                                          std::string kcname, int arity) const {
      bool found = true;
      if (cache_.size() <= category_index) {
        found = false;
      } else if (cache_.is_null(category_index)) {
        found = false;
      }
      if (!found) {
        std::string nm = get_data_data_set_name(
            kcname, arity, TypeTraits::HDF5Traits::get_name(), false);
        cache_.resize(
            std::max(cache_.size(), static_cast<size_t>(category_index + 1)),
            NULL);
        cache_.replace(category_index, new DS());
        cache_[category_index].set(file, nm);
      }
      return cache_[category_index];
    }
  };
  class KeyNameDataSetCache {
    // category, type, per_frame
    typedef HDF5DataSetCacheD<StringTraits, 1> DS;
    typedef boost::ptr_vector<boost::nullable<DS> > PVDS;
    typedef boost::array<PVDS, 2> Pair;
    mutable std::vector<Pair> cache_;

   public:
    HDF5DataSetCacheD<StringTraits, 1>& get(HDF5::Group file, Category cat,
                                            std::string kcname,
                                            unsigned int type_index,
                                            std::string type_name,
                                            bool per_frame) const {
      int pfi = per_frame ? 1 : 0;
      bool found = true;
      if (cache_.size() <= cat.get_index()) {
        found = false;
      } else if (cache_[cat.get_index()][pfi].size() <= type_index) {
        found = false;
      } else if (cache_[cat.get_index()][pfi].is_null(type_index)) {
        found = false;
      }
      if (!found) {
        std::string nm =
            get_key_list_data_set_name(kcname, type_name, per_frame);
        cache_.resize(
            std::max(cache_.size(), static_cast<size_t>(cat.get_index() + 1)));
        cache_[cat.get_index()][pfi]
            .resize(std::max(cache_[cat.get_index()][pfi].size(),
                             static_cast<size_t>(type_index + 1)),
                    NULL);
        cache_[cat.get_index()][pfi].replace(type_index, new DS());
        cache_[cat.get_index()][pfi][type_index].set(file, nm);
      }
      return cache_[cat.get_index()][pfi][type_index];
    }
  };
  mutable Ints max_cache_;
  mutable RMF_LARGE_UNORDERED_SET<std::string> known_data_sets_;
  KeyNameDataSetCache key_name_data_sets_;
  RMF_FOREACH_BACKWARDS_TYPE(RMF_HDF5_SHARED_DATA_TYPE);

  template <class TypeTraits>
  HDF5DataSetCacheD<StringTraits, 1>& get_key_list_data_set(
      Category cat, bool per_frame) const {
    return key_name_data_sets_.get(
        file_, cat, get_name(cat), TypeTraits::HDF5Traits::get_index(),
        TypeTraits::HDF5Traits::get_name(), per_frame);
  }

  template <class TypeTraits>
  HDF5DataSetCacheD<TypeTraits, 2>& get_data_data_set(
      unsigned int category_index, int arity) const {
    return get_data_set_i(TypeTraits(), category_index, arity);
  }
  template <class TypeTraits>
  HDF5DataSetCacheD<TypeTraits, 3>& get_per_frame_data_data_set(
      unsigned int category_index, int arity) const {
    return get_per_frame_data_set_i(TypeTraits(), category_index, arity);
  }

  enum Indexes {
    TYPE = 0,
    CHILD = 1,
    SIBLING = 2,
    FIRST_KEY = 3
  };

  unsigned int get_index(unsigned int category_index) const {
    return category_index + FIRST_KEY;
  }
  void check_node(NodeID node) const;

  unsigned int get_column_maximum(unsigned int category_index) const {
    if (max_cache_.size() > category_index && max_cache_[category_index] > -2) {
      return max_cache_[category_index];
    }
    HDF5::DataSetIndexD<2> sz = node_data_.get_size();
    int mx = -1;
    int index = get_index(category_index);
    for (unsigned int i = 0; i < sz[0]; ++i) {
      mx = std::max(mx, node_data_.get_value(HDF5::DataSetIndexD<2>(i, index)));
    }
    max_cache_.resize(
        std::max(max_cache_.size(), static_cast<size_t>(category_index + 1)),
        -2);
    max_cache_[category_index] = mx;
    return mx;
  }

  template <class TypeTraits>
  typename TypeTraits::Type get_value(FrameID frame, NodeID node,
                                      ID<TypeTraits> k) const {
    int category_index = get_category_index(get_category(k));
    if (category_index == -1) {
      return TypeTraits::get_null_value();
    }
    int key_index = get_key_index(k, frame);
    if (key_index != -1) {
      typename TypeTraits::Type ret =
          get_value_impl<TypeTraits>(node, category_index, key_index, frame);
      return ret;
    } else {
      return TypeTraits::get_null_value();
    }
  }
  template <class TypeTraits>
  void set_value(FrameID frame, NodeID node, ID<TypeTraits> k,
                 typename TypeTraits::Type v) {
    int category_index = get_category_index_create(get_category(k));
    int key_index = get_key_index_create(k, frame);
    set_value_impl<TypeTraits>(node, category_index, key_index, frame, v);
  }

  template <class TypeTraits>
  typename TypeTraits::Type get_value_impl(NodeID node,
                                           unsigned int category_index,
                                           unsigned int key_index,
                                           FrameID frame) const {
    int vi = get_index_from_cache(node, category_index);
    if (IndexTraits::get_is_null_value(vi)) {
      int index = get_index(category_index);
      HDF5::DataSetIndexD<2> nsz = node_data_.get_size();
      // deal with nodes added for sets
      if (nsz[0] <= static_cast<unsigned int>(node.get_index())) {
        return TypeTraits::get_null_value();
      }
      if (nsz[1] <= static_cast<hsize_t>(index)) {
        return TypeTraits::get_null_value();
      } else {
        vi = node_data_.get_value(
            HDF5::DataSetIndexD<2>(node.get_index(), index));
      }
      if (IndexTraits::get_is_null_value(vi)) {
        return TypeTraits::get_null_value();
      } else {
        add_index_to_cache(node, category_index, vi);
      }
    }
    {
      if (frame != ALL_FRAMES) {
        HDF5DataSetCacheD<TypeTraits, 3>& ds =
            get_per_frame_data_data_set<TypeTraits>(category_index, 1);
        HDF5::DataSetIndexD<3> sz = ds.get_size();
        if (static_cast<hsize_t>(vi) >= sz[0] ||
            static_cast<hsize_t>(key_index) >= sz[1] ||
            (static_cast<unsigned int>(frame.get_index()) >= sz[2])) {
          return TypeTraits::get_null_value();
        } else {
          return ds.get_value(
              HDF5::DataSetIndexD<3>(vi, key_index, frame.get_index()));
        }
      } else {
        HDF5DataSetCacheD<TypeTraits, 2>& ds =
            get_data_data_set<TypeTraits>(category_index, 1);
        HDF5::DataSetIndexD<2> sz = ds.get_size();
        if (static_cast<hsize_t>(vi) >= sz[0] ||
            static_cast<hsize_t>(key_index) >= sz[1]) {
          return TypeTraits::get_null_value();
        } else {
          return ds.get_value(HDF5::DataSetIndexD<2>(vi, key_index));
        }
      }
    }
  }
  template <class TypeTraits>
  unsigned int get_number_of_frames(unsigned int category_index) const {
    HDF5DataSetCacheD<TypeTraits, 3>& ds =
        get_per_frame_data_data_set<TypeTraits>(category_index, 1);
    HDF5::DataSetIndexD<3> sz = ds.get_size();
    return sz[2];
  }
  int get_index_from_cache(NodeID node, unsigned int category_index) const {
    if (index_cache_.size() <= static_cast<unsigned int>(node.get_index()))
      return -1;
    else if (index_cache_[node.get_index()].size() <= category_index) {
      return -1;
    }
    return index_cache_[node.get_index()][category_index];
  }
  void add_index_to_cache(NodeID node, unsigned int category_index,
                          int index) const {
    if (index_cache_.size() <= static_cast<unsigned int>(node.get_index())) {
      index_cache_.resize(node.get_index() + 1, std::vector<int>());
    }
    if (index_cache_[node.get_index()].size() <= category_index) {
      index_cache_[node.get_index()].resize(category_index + 1, -1);
    }
    index_cache_[node.get_index()][category_index] = index;
  }

  int get_index_set(NodeID node, unsigned int category_index) {
    int vi = get_index_from_cache(node, category_index);
    if (vi == -1) {
      unsigned int index = get_index(category_index);
      HDF5::DataSetIndexD<2> nsz = node_data_.get_size();
      RMF_USAGE_CHECK(nsz[0] > static_cast<unsigned int>(node.get_index()),
                      "Invalid node used");
      if (nsz[1] <= index) {
        HDF5::DataSetIndexD<2> newsz = nsz;
        newsz[1] = index + 1;
        node_data_.set_size(newsz);
      }
      // now it is big enough
      // make sure the target table is there
      /*if (!get_has_data_set(nm)) {
         file_.add_data_set<TypeTraits>(nm, (per_frame?3:2));
         }*/
      // now we have the index and the data set is there
      vi =
          node_data_.get_value(HDF5::DataSetIndexD<2>(node.get_index(), index));
      if (IndexTraits::get_is_null_value(vi)) {
        vi = get_column_maximum(category_index) + 1;
        node_data_.set_value(HDF5::DataSetIndexD<2>(node.get_index(), index),
                             vi);
        max_cache_[category_index] = vi;
      }
      add_index_to_cache(node, category_index, vi);
    }
    return vi;
  }

  template <class TypeTraits>
  void make_fit(HDF5DataSetCacheD<TypeTraits, 3>& ds, int vi,
                unsigned int key_index, unsigned int frame) {
    HDF5::DataSetIndexD<3> sz = ds.get_size();
    bool delta = false;
    if (sz[0] <= static_cast<hsize_t>(vi)) {
      sz[0] = vi + 1;
      delta = true;
    }
    if (sz[1] <= static_cast<hsize_t>(key_index)) {
      sz[1] = key_index + 1;
      delta = true;
    }
    if (static_cast<unsigned int>(sz[2]) <= frame) {
      sz[2] = std::max(frame + 1, frames_hint_);
      delta = true;
    }
    if (delta) {
      ds.set_size(sz);
    }
  }
  template <class TypeTraits>
  void make_fit(HDF5DataSetCacheD<TypeTraits, 2>& ds, int vi,
                unsigned int key_index) {
    HDF5::DataSetIndexD<2> sz = ds.get_size();
    bool delta = false;
    if (sz[0] <= static_cast<hsize_t>(vi)) {
      sz[0] = vi + 1;
      delta = true;
    }
    if (sz[1] <= static_cast<hsize_t>(key_index)) {
      sz[1] = key_index + 1;
      delta = true;
    }
    if (delta) {
      ds.set_size(sz);
    }
  }
  template <class TypeTraits>
  void set_value_impl(NodeID node, unsigned int category_index,
                      unsigned int key_index, FrameID frame,
                      typename TypeTraits::Type v) {
    RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(v),
                    "Cannot write sentry value to an RMF file.");
    int vi = get_index_set(node, category_index);
    if (frame != ALL_FRAMES) {
      HDF5DataSetCacheD<TypeTraits, 3>& ds =
          get_per_frame_data_data_set<TypeTraits>(category_index, 1);
      make_fit(ds, vi, key_index, frame.get_index());
      ds.set_value(HDF5::DataSetIndexD<3>(vi, key_index, frame.get_index()), v);
    } else {
      HDF5DataSetCacheD<TypeTraits, 2>& ds =
          get_data_data_set<TypeTraits>(category_index, 1);
      make_fit(ds, vi, key_index);
      ds.set_value(HDF5::DataSetIndexD<2>(vi, key_index), v);
    }
    /*RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                           "Stored " << v << " but got "
                           << get_value(node, k, frame));*/
  }

  template <class TypeTraits>
  unsigned int add_key_impl(Category cat, std::string name, bool per_frame) {
    // check that it is unique
    {
      HDF5DataSetCacheD<StringTraits, 1>& nameds =
          get_key_list_data_set<TypeTraits>(cat, per_frame);
      unsigned int sz = nameds.get_size()[0];
      HDF5::DataSetIndexD<1> index;
      for (unsigned int i = 0; i < sz; ++i) {
        index[0] = i;
        RMF_USAGE_CHECK(
            nameds.get_value(index) != name,
            internal::get_error_message("Attribute name ", name,
                                        " already taken for that type."));
      }
    }
    HDF5DataSetCacheD<StringTraits, 1>& nameds =
        get_key_list_data_set<TypeTraits>(cat, per_frame);
    HDF5::DataSetIndexD<1> sz = nameds.get_size();
    int ret_index = sz[0];
    ++sz[0];
    nameds.set_size(sz);
    --sz[0];
    nameds.set_value(sz, name);
    return ret_index;
  }

  void initialize_keys(int i);
  void initialize_free_nodes();
  void initialize_categories();

  NodeID get_first_child(NodeID node) const;
  NodeID get_sibling(NodeID node) const;
  void set_first_child(NodeID node, NodeID child);
  void set_sibling(NodeID node, NodeID sibling);
  void close_things();

  // opens the file in file_name_
  // @param create - whether to create the file or just open it
  void open_things(bool create, bool read_only);

  NodeID get_linked(NodeID node) const;
  unsigned int add_category_impl(std::string name);
  std::string get_category_name_impl(unsigned int category_index) const {
    RMF_USAGE_CHECK(category_names_.get_size()[0] > category_index,
                    "No such category.");
    return category_names_.get_value(category_index);
  }

  int get_category_index(Category cat) const {
    CategoryDataMap::const_iterator it = category_data_map_.find(cat);
    return it->second.index;
  }
  int get_category_index_create(Category cat) {
    CategoryDataMap::iterator it = category_data_map_.find(cat);
    if (it->second.index == -1) {
      it->second.index = add_category_impl(it->second.name);
    }
    return it->second.index;
  }

  template <class TypeTraits>
  int get_key_index(ID<TypeTraits> key, bool per_frame) const {
    KeyDataMap::const_iterator it = key_data_map_.find(key.get_index());
    if (per_frame) {
      return it->second.per_frame_index;
    } else {
      return it->second.static_index;
    }
  }

  template <class TypeTraits>
  int get_key_index(ID<TypeTraits> key, FrameID frame) const {
    return get_key_index(key, frame != ALL_FRAMES);
  }
  template <class TypeTraits>
  int get_key_index_create(ID<TypeTraits> key, FrameID frame) {
    KeyDataMap::iterator it = key_data_map_.find(key.get_index());
    if (frame != ALL_FRAMES) {
      if (it->second.per_frame_index == -1) {
        int index = add_key_impl<TypeTraits>(
            get_category(key), key_data_map_[key.get_index()].name, true);
        it->second.per_frame_index = index;
        return index;
      } else {
        return it->second.per_frame_index;
      }
    } else {
      if (it->second.static_index == -1) {
        int index = add_key_impl<TypeTraits>(
            get_category(key), key_data_map_[key.get_index()].name, false);
        it->second.static_index = index;
        return index;
      } else {
        return it->second.static_index;
      }
    }
  }

 public:
  NodeID add_node(std::string name, NodeType type);

  template <class TypeTraits>
  typename TypeTraits::Type get_loaded_value(NodeID node,
                                             ID<TypeTraits> k) const {
    return get_value(get_loaded_frame(), node, k);
  }
  template <class TypeTraits>
  typename TypeTraits::Type get_static_value(NodeID node,
                                             ID<TypeTraits> k) const {
    return get_value(ALL_FRAMES, node, k);
  }
  template <class TypeTraits>
  void set_loaded_value(NodeID node, ID<TypeTraits> k,
                        typename TypeTraits::Type v) {
    set_value(get_loaded_frame(), node, k, v);
  }
  template <class TypeTraits>
  void set_static_value(NodeID node, ID<TypeTraits> k,
                        typename TypeTraits::Type v) {
    set_value(ALL_FRAMES, node, k, v);
  }

  template <class TypeTraits>
  std::vector<ID<TypeTraits> > get_keys(Category cat, TypeTraits) const {
    std::vector<ID<TypeTraits> > ret;
    typename NameKeyMap::const_iterator oit = name_key_map_.find(cat);
    if (oit == name_key_map_.end()) return ret;
    RMF_FOREACH(NameKeyInnerMap::const_reference rt, oit->second) {
      if (key_data_map_.find(rt.second)->second.type_index ==
          TypeTraits::HDF5Traits::get_index()) {
        ret.push_back(ID<TypeTraits>(rt.second));
      }
    }
    return ret;
  }

  template <class TypeTraits>
  ID<TypeTraits> get_key(Category category, std::string name, TypeTraits) {
    NameKeyInnerMap::iterator it = name_key_map_[category].find(name);
    if (it == name_key_map_[category].end()) {
      int id = key_data_map_.size();
      name_key_map_[category][name] = id;
      key_data_map_[id].name = name;
      key_data_map_[id].per_frame_index = -1;
      key_data_map_[id].static_index = -1;
      key_data_map_[id].type_index = TypeTraits::HDF5Traits::get_index();
      key_data_map_[id].category = category;
      return ID<TypeTraits>(id);
    } else {
      RMF_USAGE_CHECK(
          key_data_map_.find(it->second)->second.type_index ==
              TypeTraits::HDF5Traits::get_index(),
          "Key already defined with a different type in that category.");
      return ID<TypeTraits>(it->second);
    }
  }

  template <class Traits>
  void initialize_keys(Category cat, std::string name, Traits) {
    RMF_TRACE("Checking for " << name << " keys.");
    RMF_UNUSED(name);
    for (int pf = 0; pf < 2; ++pf) {
      bool per_frame = (pf == 1);
      HDF5DataSetCacheD<StringTraits, 1>& nameds =
          get_key_list_data_set<Traits>(cat, per_frame);
      HDF5::DataSetIndexD<1> sz = nameds.get_size();
      for (unsigned int j = 0; j < sz[0]; ++j) {
        std::string name = nameds.get_value(HDF5::DataSetIndexD<1>(j));
        int id;
        NameKeyInnerMap::iterator it = name_key_map_[cat].find(name);
        if (it == name_key_map_[cat].end()) {
          id = key_data_map_.size();
          name_key_map_[cat][name] = id;
          key_data_map_[id].name = name;
          key_data_map_[id].type_index = Traits::HDF5Traits::get_index();
          key_data_map_[id].per_frame_index = -1;
          key_data_map_[id].static_index = -1;
          key_data_map_[id].category = cat;
        } else {
          id = it->second;
        }
        if (per_frame) {
          key_data_map_[id].per_frame_index = j;
        } else {
          key_data_map_[id].static_index = j;
        }
      }
    }
  }

  template <class Traits>
  std::string get_name(ID<Traits> k) const {
    return key_data_map_.find(k.get_index())->second.name;
  }
  template <class Traits>
  Category get_category(ID<Traits> key) const {
    return key_data_map_.find(key.get_index())->second.category;
  }

  void set_name(FrameID i, std::string str);
  HDF5::Group get_group() const { return file_; }
  void flush();

  /**
     constructs HDF5SharedData for the RMF file g, either creating
     or opening the file according to the value of create.

     @param g - path to file
     @param create - whether to create the file or just open it
     @exception RMF::IOException if couldn't create / open file
                or bad file format
   */
  HDF5SharedData(std::string g, bool create, bool read_only);
  ~HDF5SharedData();
  std::string get_name(NodeID node) const;
  NodeType get_type(NodeID node) const;
  NodeID add_child(NodeID node, std::string name, NodeType t);
  void add_child(NodeID node, NodeID child_node);
  NodeIDs get_children(NodeID node) const;
  unsigned int get_number_of_frames() const;
  unsigned int get_number_of_nodes() const { return node_names_.get_size()[0]; }

  Categories get_categories() const;
  Category get_category(std::string name);
  std::string get_name(Category kc) const {
    return category_data_map_.find(kc)->second.name;
  }

  std::string get_description() const;
  void set_description(std::string str);

  std::string get_producer() const;
  void set_producer(std::string str);

  std::string get_loaded_frame_name() const;

  bool get_supports_locking() const { return false; }
  void reload();
  void set_loaded_frame(FrameID frame);

  std::string get_file_type() const { return "HDF5 version 1"; }

  FrameID add_frame(std::string name, FrameType /*t*/) {
    // frame types not supported in RMF files right now
    unsigned int cindex = get_number_of_frames();
    FrameID index(cindex);
    set_name(index, name);
    return index;
  }
  void add_child_frame(FrameID /*child_node*/) {}
  FrameType get_loaded_frame_type() const { return FRAME; }
  FrameIDs get_children(FrameID node) const {
    unsigned int cindex;
    if (node == ALL_FRAMES)
      cindex = 0;
    else
      cindex = node.get_index() + 1;
    if (cindex <= get_number_of_frames()) {
      return FrameIDs(1, FrameID(cindex));
    } else {
      return FrameIDs();
    }
  }
};

}  // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_HDF_5SHARED_DATA_H */
