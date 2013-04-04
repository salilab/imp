/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF_5SHARED_DATA_H
#define RMF_INTERNAL_HDF_5SHARED_DATA_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/compiler_macros.h>
#include <RMF/HDF5/Group.h>
#include <RMF/HDF5/File.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/constants.h>
#include <RMF/internal/map.h>
#include <RMF/internal/set.h>

#include "HDF5DataSetCacheD.h"
#include "HDF5DataSetCache2D.h"
#include "HDF5DataSetCache1D.h"
#include "HDF5DataSetCache3D.h"
#include "names.h"
#include <boost/array.hpp>
#include <hdf5.h>
#include <algorithm>
#include <boost/ptr_container/ptr_vector.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {


namespace hdf5_backend {

#define RMF_HDF5_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue,  \
                                  PassValues, ReturnValues)                \
  DataDataSetCache2D<Ucname##Traits> lcname##_data_sets_;                  \
  DataDataSetCache3D<Ucname##Traits> per_frame_##lcname##_data_sets_;      \
  HDF5DataSetCacheD<Ucname##Traits, 2>&                                    \
  get_data_set_i(Ucname##Traits,                                           \
                 unsigned int kc, int arity) const {                       \
    return lcname##_data_sets_.get(file_, kc,                              \
                                   get_category_name_impl(kc), arity);     \
  }                                                                        \
  HDF5DataSetCacheD<Ucname##Traits, 3>&                                    \
  get_per_frame_data_set_i(Ucname##Traits,                                 \
                           unsigned int kc, int arity) const {             \
    return per_frame_##lcname##_data_sets_.get(file_, kc,                  \
                                               get_category_name_impl(kc), \
                                               arity);                     \
  }


#define RMF_HDF5_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,  \
                             PassValues, ReturnValues)                \
  Ucname##Traits::Type get_value(unsigned int node,                   \
                                 Key<Ucname##Traits> k) const {       \
    return get_value_helper<Ucname##Traits>(node, k);                 \
  }                                                                   \
  Ucname##Traits::Type get_value_frame(unsigned int ,                 \
                                       Key<Ucname##Traits> ) const {  \
    return Ucname##Traits::get_null_value();                          \
  }                                                                   \
  virtual Ucname##Traits::Types get_all_values(unsigned int node,     \
                                       Key<Ucname##Traits> k)         \
    const RMF_OVERRIDE {                                              \
    return get_all_values_helper<Ucname##Traits>(node, k);            \
  }                                                                   \
  void set_value(unsigned int node,                                   \
                 Key<Ucname##Traits> k,                               \
                 Ucname##Traits::Type v) {                            \
    return set_value_helper<Ucname##Traits>(node, k, v);              \
  }                                                                   \
  void set_value_frame(unsigned int ,                                 \
                       Key<Ucname##Traits> ,                            \
                       Ucname##Traits::Type ) {                         \
    RMF_THROW(Message("Not supported in this hdf5_backend"), IOException); \
  }                                                                   \
  bool get_has_frame_value(unsigned int node,                         \
                           Key<Ucname##Traits> k) const {             \
    return get_has_frame_value_helper(node, k);                       \
  }                                                                   \
  std::vector<Key<Ucname##Traits> >                                   \
  get_##lcname##_keys(Category category) {                            \
    return get_keys_helper<Ucname##Traits>(category);                 \
  }                                                                   \
  Key<Ucname##Traits>                                                 \
  get_##lcname##_key(Category category,                               \
                     std::string name) {                              \
    return get_key_helper<Ucname##Traits>(category, name);            \
  }                                                                   \
  std::string get_name(Key<Ucname##Traits> k) const {                 \
    return key_data_map_.find(k.get_id())->second.name;               \
  }                                                                   \
  Category get_category(Key<Ucname##Traits> key) const {              \
    return key_data_map_.find(key.get_id())->second.category;         \
  }



class HDF5SharedData: public internal::SharedData {
  // indexed first by per frame, then by
  // TypeInfo::get_index() then by ID
  // then by key.get_index()
  mutable HDF5::Group file_;
  bool read_only_;
  HDF5DataSetCacheD<StringTraits, 1> node_names_;
  HDF5DataSetCacheD<StringTraits, 1> frame_names_;
  HDF5DataSetCacheD<StringTraits, 1> category_names_;
  boost::array<HDF5DataSetCacheD<IndexTraits, 2>, 4> node_data_;
  Ints free_ids_;
  unsigned int frames_hint_;

  struct CategoryData {
    int index;
    std::string name;
  };

  typedef internal::map<Category, CategoryData> CategoryDataMap;
  CategoryDataMap category_data_map_;
  typedef internal::map<std::string, Category> NameCategoryMap;
  NameCategoryMap name_category_map_;

  struct KeyData {
    int static_index, per_frame_index;
    std::string name;
    Category category;
    int type_index;
  };

  typedef internal::map<unsigned int, KeyData> KeyDataMap;
  KeyDataMap key_data_map_;
  typedef internal::map<std::string, unsigned int> NameKeyInnerMap;
  typedef internal::map<Category, NameKeyInnerMap > NameKeyMap;
  NameKeyMap name_key_map_;


  Category link_category_;
  Key<NodeIDTraits> link_key_;

  // caches
  typedef std::vector< std::vector<int> > IndexCache;
  mutable boost::array<IndexCache, 4> index_cache_;

  /*                 HDF5DataSetCreationPropertiesD<TypeTraits, D> props;
            if (D==3) {
              props.set_chunk_size(HDF5DataSetIndexD<D>(256, 4, 100));
            } else if (D==2) {
              props.set_chunk_size(HDF5DataSetIndexD<D>(256, 4));

            props.set_compression(GZIP_COMPRESSION);


   */
  template <class TypeTraits>
  class DataDataSetCache3D {
    typedef HDF5DataSetCacheD<TypeTraits, 3> DS;
    mutable boost::ptr_vector<boost::nullable<DS> > cache_;
    unsigned int frame_;
public:
    DataDataSetCache3D(): frame_(0) {
    }
    HDF5DataSetCacheD<TypeTraits, 3>& get(HDF5::Group    file,
                                          unsigned int category_index,
                                          std::string  kcname,
                                          int          arity) const {
      bool found = true;
      if (cache_.size() <= category_index) {
        found = false;
      } else if (cache_.is_null(category_index)) {
        found = false;
      }
      if (!found) {
        std::string nm = get_data_data_set_name(kcname, arity,
                                                TypeTraits::HDF5Traits::get_name(),
                                                true);
        cache_.resize(std::max(cache_.size(),
                               static_cast<size_t>(category_index + 1)),
                      NULL);
        cache_.replace(category_index, new DS());
        cache_[category_index].set_current_frame(frame_);
        cache_[category_index].set(file, nm);
      }
      return cache_[category_index];
    }
    void set_current_frame(unsigned int f) {
      frame_ = f;
      for (unsigned int i = 0; i < cache_.size(); ++i) {
        if (!cache_.is_null(i)) {
          cache_[i].set_current_frame(f);
        }
      }
    }
  };
  template <class TypeTraits>
  class DataDataSetCache2D {
    typedef HDF5DataSetCacheD<TypeTraits, 2> DS;
    mutable boost::ptr_vector<boost::nullable<DS> > cache_;
public:
    HDF5DataSetCacheD<TypeTraits, 2>& get(HDF5::Group    file,
                                          unsigned int category_index,
                                          std::string  kcname,
                                          int          arity) const {
      bool found = true;
      if (cache_.size() <= category_index) {
        found = false;
      } else if (cache_.is_null(category_index)) {
        found = false;
      }
      if (!found) {
        std::string nm = get_data_data_set_name(kcname, arity,
                                                TypeTraits::HDF5Traits::get_name(),
                                                false);
        cache_.resize(std::max(cache_.size(),
                               static_cast<size_t>(category_index + 1)),
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
    mutable std::vector<Pair > cache_;
public:
    HDF5DataSetCacheD<StringTraits, 1>& get(HDF5::Group    file,
                                            Category     cat,
                                            std::string  kcname,
                                            unsigned int type_index,
                                            std::string  type_name,
                                            bool         per_frame) const {
      int pfi = per_frame ? 1 : 0;
      bool found = true;
      if (cache_.size() <= cat.get_id()) {
        found = false;
      } else if (cache_[cat.get_id()][pfi].size() <= type_index) {
        found = false;
      } else if (cache_[cat.get_id()][pfi].is_null(type_index)) {
        found = false;
      }
      if (!found) {
        std::string nm = get_key_list_data_set_name(kcname,
                                                    type_name,
                                                    per_frame);
        cache_.resize(std::max(cache_.size(),
                               static_cast<size_t>(cat.get_id() + 1)));
        cache_[cat.get_id()][pfi].resize(std::max(cache_[cat.get_id()]
                                                  [pfi].size(),
                                                  static_cast<size_t>(type_index + 1)),
                                         NULL);
        cache_[cat.get_id()][pfi].replace(type_index, new DS());
        cache_[cat.get_id()][pfi][type_index].set(file, nm);
      }
      return cache_[cat.get_id()][pfi][type_index];
    }
  };
  mutable Ints max_cache_;
  mutable internal::set<std::string> known_data_sets_;
  KeyNameDataSetCache key_name_data_sets_;
  RMF_FOREACH_TYPE(RMF_HDF5_SHARED_DATA_TYPE);

  template <class TypeTraits>
  HDF5DataSetCacheD<StringTraits, 1>&
  get_key_list_data_set(Category cat,
                        bool     per_frame) const {
    return key_name_data_sets_.get(file_, cat,
                                   get_category_name(cat),
                                   TypeTraits::HDF5Traits::get_index(),
                                   TypeTraits::HDF5Traits::get_name(),
                                   per_frame);
  }

  template <class TypeTraits>
  HDF5DataSetCacheD<TypeTraits, 2>&
  get_data_data_set(unsigned int category_index, int arity) const {
    return get_data_set_i(TypeTraits(), category_index, arity);
  }
  template <class TypeTraits>
  HDF5DataSetCacheD<TypeTraits, 3>&
  get_per_frame_data_data_set(unsigned int category_index,
                              int          arity) const {
    return get_per_frame_data_set_i(TypeTraits(), category_index, arity);
  }

  enum Indexes {TYPE=0, CHILD=1, SIBLING=2, FIRST_KEY=3};

  unsigned int get_index(int Arity, unsigned int category_index) const {
    switch (Arity) {
    case 1:
      return category_index + FIRST_KEY;
    default:
      return 1 + Arity + category_index;
    }
  }
  void check_node(unsigned int node) const;
  template <int Arity>
  unsigned int get_column_maximum(unsigned int category_index) const {
    if (max_cache_.size() > category_index
        && max_cache_[category_index] > -2) {
      return max_cache_[category_index];
    }
    HDF5::DataSetIndexD<2> sz = node_data_[Arity - 1].get_size();
    int mx = -1;
    int index = get_index(Arity, category_index);
    for (unsigned int i = 0; i < sz[0]; ++i) {
      mx = std::max(mx,
                    node_data_[Arity - 1]
                    .get_value(HDF5::DataSetIndexD<2>(i, index)));
    }
    max_cache_.resize(std::max(max_cache_.size(),
                               static_cast<size_t>(category_index + 1)), -2);
    max_cache_[category_index] = mx;
    return mx;
  }

  template <class TypeTraits>
  bool get_has_frame_value_helper(unsigned int    node,
                                  Key<TypeTraits> k) const {
    int category_index = get_category_index(get_category(k));
    if (category_index == -1) {
      return false;
    }
    int key_index = get_key_index(k);
    if (key_index == -1) {
      return false;
    }
    typename TypeTraits::Type ret = get_value_impl<TypeTraits>(node,
                                                               category_index,
                                                               key_index,
                                                                 get_current_frame()
                                                               != ALL_FRAMES,
                                                                 get_current_frame());
    if ( TypeTraits::get_is_null_value(ret)) {
      return false;
    }
    return true;
  }
  template <class TypeTraits>
  typename TypeTraits::Type get_value_helper(unsigned int    node,
                                             Key<TypeTraits> k) const {
    int category_index = get_category_index(get_category(k));
    if (category_index == -1) {
      return TypeTraits::get_null_value();
    }
    int key_index = get_key_index(k);
    bool per_frame = (get_current_frame() != ALL_FRAMES);
    if (key_index != -1) {
      typename TypeTraits::Type ret = get_value_impl<TypeTraits>(node,
                                                                 category_index,
                                                                 key_index,
                                                                 per_frame,
                                                                 get_current_frame());
      if ( !TypeTraits::get_is_null_value(ret)) {
        return ret;
      }
    }
    if (per_frame) {
      // check for a static value
      key_index = get_key_index(k, false);
      return get_value_impl<TypeTraits>(node,
                                        category_index,
                                        key_index,
                                        false,
                                                                 get_current_frame());
    } else {
      return TypeTraits::get_null_value();
    }
  }
  template <class TypeTraits>
  typename TypeTraits::Types get_all_values_helper(unsigned int    node,
                                                   Key<TypeTraits> k) const {
    int category_index = get_category_index(get_category(k));
    if (category_index == -1) {
      return typename TypeTraits::Types();
    }
    // we always want the per-frame variant
    int key_index = get_key_index(k, true);
    if (key_index == -1) {
      return typename TypeTraits::Types();
    }
    return get_all_values_impl<TypeTraits>(node,
                                           category_index,
                                           key_index);
  }
  template <class TypeTraits>
  void set_value_helper(unsigned int              node,
                        Key<TypeTraits>           k,
                        typename TypeTraits::Type v) {
    int category_index = get_category_index_create(get_category(k));
    int key_index = get_key_index_create(k);
    set_value_impl<TypeTraits>(node,
                               category_index,
                               key_index,
                               get_current_frame() != ALL_FRAMES,
                               v, get_current_frame());
  }
  template <class TypeTraits>
  std::vector<Key<TypeTraits> >
  get_keys_helper(Category category) {
    return get_keys_impl<TypeTraits>(category);
  }
  template <class TypeTraits>
  Key<TypeTraits>
  get_key_helper(Category    category,
                 std::string name) {
    NameKeyInnerMap::iterator it
      = name_key_map_[category].find(name);
    if (it == name_key_map_[category].end()) {
      int id = key_data_map_.size();
      name_key_map_[category][name] = id;
      key_data_map_[id].name = name;
      key_data_map_[id].per_frame_index = -1;
      key_data_map_[id].static_index = -1;
      key_data_map_[id].type_index = TypeTraits::HDF5Traits::get_index();
      key_data_map_[id].category = category;
      return Key<TypeTraits>(id);
    } else {
      RMF_USAGE_CHECK(key_data_map_.find(it->second)->second.type_index
                      == TypeTraits::HDF5Traits::get_index(),
                      "Key already defined with a different type in that category.");
      return Key<TypeTraits>(it->second);
    }
  }

  template <class TypeTraits>
  typename TypeTraits::Types get_all_values_impl(unsigned int node,
                                                 unsigned int category_index,
                                                 unsigned int key_index)
  const {
    int vi = get_index_from_cache<1>(node, category_index);
    if (IndexTraits::get_is_null_value(vi)) {
      int index = get_index(1, category_index);
      HDF5::DataSetIndexD<2> nsz = node_data_[1 - 1].get_size();
      RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                      "Invalid node used");
      if (nsz[1] <= static_cast<hsize_t>(index)) {
        return typename TypeTraits::Types();
      } else {
        vi = node_data_[1 - 1].get_value(HDF5::DataSetIndexD<2>(node, index));
      }
      if (IndexTraits::get_is_null_value(vi)) {
        return typename TypeTraits::Types();
      } else {
        add_index_to_cache<1>(node, category_index, vi);
      }
    }
    {
      HDF5DataSetCacheD<TypeTraits, 3> &ds
        = get_per_frame_data_data_set<TypeTraits>(category_index,
                                                  1);
      HDF5::DataSetIndexD<3> sz = ds.get_size();
      if (static_cast<hsize_t>(vi) >= sz[0]
          || static_cast<hsize_t>(key_index) >= sz[1]) {
        return typename TypeTraits::Types();
      } else {
        return ds.get_row(HDF5::DataSetIndexD<2>(vi, key_index));
      }
    }
  }

  template <class TypeTraits>
  typename TypeTraits::Type get_value_impl(unsigned int node,
                                           unsigned int category_index,
                                           unsigned int key_index,
                                           bool         per_frame,
                                           unsigned int frame) const {
    int vi = get_index_from_cache<1>(node, category_index);
    if (IndexTraits::get_is_null_value(vi)) {
      int index = get_index(1, category_index);
      HDF5::DataSetIndexD<2> nsz = node_data_[1 - 1].get_size();
      // deal with nodes added for sets
      if (static_cast<unsigned int>(nsz[0]) <= node) {
        return TypeTraits::get_null_value();
      }
      if (nsz[1] <= static_cast<hsize_t>(index)) {
        return TypeTraits::get_null_value();
      } else {
        vi = node_data_[1 - 1].get_value(HDF5::DataSetIndexD<2>(node, index));
      }
      if (IndexTraits::get_is_null_value(vi)) {
        return TypeTraits::get_null_value();
      } else {
        add_index_to_cache<1>(node, category_index, vi);
      }
    }
    {
      if (per_frame) {
        HDF5DataSetCacheD<TypeTraits, 3> &ds
          = get_per_frame_data_data_set<TypeTraits>(category_index,
                                                    1);
        HDF5::DataSetIndexD<3> sz = ds.get_size();
        if (static_cast<hsize_t>(vi) >= sz[0]
            || static_cast<hsize_t>(key_index) >= sz[1]
            || (frame >= static_cast<unsigned int>(sz[2]))) {
          return TypeTraits::get_null_value();
        } else {
          return ds.get_value(HDF5::DataSetIndexD<3>(vi, key_index,
                                                   frame));
        }
      } else {
        HDF5DataSetCacheD<TypeTraits, 2> &ds
          = get_data_data_set<TypeTraits>(category_index,
                                          1);
        HDF5::DataSetIndexD<2> sz = ds.get_size();
        if (static_cast<hsize_t>(vi) >= sz[0]
            || static_cast<hsize_t>(key_index) >= sz[1]) {
          return TypeTraits::get_null_value();
        } else {
          return ds.get_value(HDF5::DataSetIndexD<2>(vi, key_index));
        }
      }
    }
  }
  template <class TypeTraits>
  unsigned int get_number_of_frames(unsigned int category_index) const {
    HDF5DataSetCacheD<TypeTraits, 3> &ds
      = get_per_frame_data_data_set<TypeTraits>(category_index,
                                                1);
    HDF5::DataSetIndexD<3> sz = ds.get_size();
    return sz[2];
  }
  template <int Arity>
  int get_index_from_cache(unsigned int node,
                           unsigned int category_index) const {
    if (index_cache_[Arity - 1].size() <= node) return -1;
    else if (index_cache_[Arity - 1][node].size() <= category_index) {
      return -1;
    }
    return index_cache_[Arity - 1][node][category_index];
  }
  template <int Arity>
  void add_index_to_cache(unsigned int node,
                          unsigned int category_index,
                          int          index) const {
    if (index_cache_[Arity - 1].size() <= node) {
      index_cache_[Arity - 1].resize(node + 1, std::vector<int>());
    }
    if (index_cache_[Arity - 1][node].size() <= category_index) {
      index_cache_[Arity - 1][node].resize(category_index + 1, -1);
    }
    index_cache_[Arity - 1][node][category_index] = index;
  }

  template <int Arity>
  int get_index_set(unsigned int node, unsigned int category_index) {
    int vi = get_index_from_cache<Arity>(node, category_index);
    if (vi == -1) {
      unsigned int index = get_index(Arity, category_index);
      HDF5::DataSetIndexD<2> nsz = node_data_[Arity - 1].get_size();
      RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                      "Invalid node used");
      if (nsz[1] <= index) {
        HDF5::DataSetIndexD<2> newsz = nsz;
        newsz[1] = index + 1;
        node_data_[Arity - 1].set_size(newsz);
      }
      // now it is big enough
      // make sure the target table is there
      /*if (!get_has_data_set(nm)) {
         file_.add_data_set<TypeTraits>(nm, (per_frame?3:2));
         }*/
      // now we have the index and the data set is there
      vi = node_data_[Arity - 1].get_value(HDF5::DataSetIndexD<2>(node, index));
      if (IndexTraits::get_is_null_value(vi)) {
        vi = get_column_maximum<Arity>(category_index) + 1;
        node_data_[Arity - 1].set_value(HDF5::DataSetIndexD<2>(node,
                                                             index), vi);
        max_cache_[category_index] = vi;
      }
      add_index_to_cache<Arity>(node, category_index, vi);
    }
    return vi;
  }

  template <class TypeTraits>
  void make_fit( HDF5DataSetCacheD<TypeTraits, 3> &ds,
                 int vi,
                 unsigned int key_index,
                 unsigned int frame) {
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
  void make_fit( HDF5DataSetCacheD<TypeTraits, 2> &ds,
                 int vi,
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
  void set_value_impl(unsigned int node,
                      unsigned int category_index,
                      unsigned int key_index,
                      bool per_frame,
                      typename TypeTraits::Type v, unsigned int frame) {
    RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(v),
                    "Cannot write sentry value to an RMF file.");
    int vi = get_index_set<1>(node, category_index);
    if (per_frame) {
      HDF5DataSetCacheD<TypeTraits, 3> &ds
        = get_per_frame_data_data_set<TypeTraits>(category_index,
                                                  1);
      make_fit(ds, vi, key_index, frame);
      ds.set_value(HDF5::DataSetIndexD<3>(vi, key_index, frame), v);
    } else {
      HDF5DataSetCacheD<TypeTraits, 2> &ds
        = get_data_data_set<TypeTraits>(category_index, 1);
      make_fit(ds, vi, key_index);
      ds.set_value(HDF5::DataSetIndexD<2>(vi, key_index), v);
    }
    /*RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                           "Stored " << v << " but got "
                           << get_value(node, k, frame));*/
  }

  template <class TypeTraits>
  unsigned int add_key_impl(Category cat,
                            std::string name, bool per_frame) {
    audit_key_name(name);
    // check that it is unique
    {
      HDF5DataSetCacheD<StringTraits, 1> &nameds
        = get_key_list_data_set<TypeTraits>(cat,
                                            per_frame);
      unsigned int sz = nameds.get_size()[0];
      HDF5::DataSetIndexD<1> index;
      for (unsigned int i = 0; i < sz; ++i) {
        index[0] = i;
        RMF_USAGE_CHECK(nameds.get_value(index) != name,
                        internal::get_error_message("Attribute name ", name,
                                          " already taken for that type."));
      }
    }
    HDF5DataSetCacheD<StringTraits, 1>& nameds
      = get_key_list_data_set<TypeTraits>(cat,
                                          per_frame);
    HDF5::DataSetIndexD<1> sz = nameds.get_size();
    int ret_index = sz[0];
    ++sz[0];
    nameds.set_size(sz);
    --sz[0];
    nameds.set_value(sz, name);
    return ret_index;
  }

  template <class TypeTraits>
  std::vector<Key<TypeTraits> >
  get_keys_impl(Category cat) {
    std::vector<Key<TypeTraits> > ret;
    typename NameKeyMap::const_iterator oit = name_key_map_.find(cat);
    if (oit == name_key_map_.end()) return ret;
    for (NameKeyInnerMap::const_iterator it
           = oit->second.begin(); it != oit->second.end(); ++it) {
      if (key_data_map_.find(it->second)->second.type_index
          == TypeTraits::HDF5Traits::get_index()) {
        ret.push_back(Key<TypeTraits>(it->second));
      }
    }
    return ret;
  }

  void initialize_keys(int i);
  void initialize_free_nodes();
  void initialize_categories();

  int get_first_child(unsigned int node) const;
  int get_sibling(unsigned int node) const;
  void set_first_child(unsigned int node, int child);
  void set_sibling(unsigned int node, int sibling);
  int add_node(std::string name, unsigned int type);
  void close_things();

  // opens the file in file_name_
  // @param create - whether to create the file or just open it
  void open_things(bool create, bool read_only);

  unsigned int get_number_of_real_nodes() const {
    return node_data_[0].get_size()[0];
  }

  unsigned int get_number_of_sets(int arity) const;
  unsigned int get_set_member(int Arity, unsigned int index,
                              int member_index) const;
  int get_linked(int node) const;
  unsigned int add_category_impl(std::string name);
  std::string get_category_name_impl(unsigned int category_index) const {
    RMF_USAGE_CHECK(category_names_.get_size()[0]
                    > category_index,
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
  int get_key_index(Key<TypeTraits> key,
                    bool            per_frame) const {
    KeyDataMap::const_iterator it = key_data_map_.find(key.get_id());
    if (per_frame) {
      return it->second.per_frame_index;
    } else {
      return it->second.static_index;
    }
  }

  template <class TypeTraits>
  int get_key_index(Key<TypeTraits> key) const {
    return get_key_index(key, get_current_frame() != ALL_FRAMES);
  }
  template <class TypeTraits>
  int get_key_index_create(Key<TypeTraits> key) {
    KeyDataMap::iterator it = key_data_map_.find(key.get_id());
    if (get_current_frame() != ALL_FRAMES) {
      if (it->second.per_frame_index == -1) {
        int index = add_key_impl<TypeTraits>(get_category(key),
                                             key_data_map_[key.get_id()].name,
                                             true);
        it->second.per_frame_index = index;
        return index;
      } else {
        return it->second.per_frame_index;
      }
    } else {
      if (it->second.static_index == -1) {
        int index = add_key_impl<TypeTraits>(get_category(key),
                                             key_data_map_[key.get_id()].name,
                                             false);
        it->second.static_index = index;
        return index;
      } else {
        return it->second.static_index;
      }
    }
  }
  void set_frame_name(int i, std::string str);

public:
  RMF_FOREACH_TYPE(RMF_HDF5_SHARED_TYPE);

  HDF5::Group get_group() const {
    return file_;
  }
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
  std::string get_name(unsigned int node) const;
  unsigned int get_type(unsigned int node) const;
  int add_child(int node, std::string name, int t);
  void add_child(int node, int child_node);
  Ints get_children(int node) const;
  void save_frames_hint(int i) {
    frames_hint_ = i;
  }
  unsigned int get_number_of_frames() const;
  Categories get_categories() const;
  Category get_category(std::string name);
  std::string get_category_name(Category kc) const {
    return category_data_map_.find(kc)->second.name;
  }

  std::string get_description() const;
  void set_description(std::string str);

  std::string get_producer() const;
  void set_producer(std::string str);


  std::string get_frame_name(int i) const;

  bool get_supports_locking() const {
    return false;
  }
  void reload();
  void set_current_frame(int frame);

  int add_child_frame(int node, std::string name, int /*t*/) {
    // frame types not supported in rmf files right now
    int index = node + 1;
    set_frame_name(index, name);
    return index;
  }
  void add_child_frame(int /*node*/, int /*child_node*/) {
  }
  Ints get_children_frame(int node) const {
    if (node != static_cast<int>(get_number_of_frames()) - 1) {
      return Ints(1, node + 1);
    } else {
      return Ints();
    }
  }

};

}   // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_HDF_5SHARED_DATA_H */
