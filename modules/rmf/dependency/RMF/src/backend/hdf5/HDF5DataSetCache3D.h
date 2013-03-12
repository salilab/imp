/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H

#include <RMF/config.h>
#include <RMF/HDF5/DataSetD.h>
#include "HDF5DataSetCacheD.h"
#include <boost/multi_array.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace hdf5_backend {

template <class TypeTraits>
class HDF5DataSetCacheD<TypeTraits, 3>/*: public boost::noncopyable*/ {
  typedef HDF5::DataSetD<typename TypeTraits::HDF5Traits, 3> DS;
  typedef boost::multi_array<typename TypeTraits::Type, 2> array_type;
  typedef typename array_type::index index;
  array_type cache_;
  HDF5::DataSetIndexD<3> extents_;
  bool dirty_;
  DS ds_;
  HDF5::Group parent_;
  std::string name_;
  unsigned int current_frame_;
  void initialize(DS ds) {
    RMF_USAGE_CHECK(!dirty_, "Trying to set a set that is already set");
    ds_ = ds;
    if (ds == DS()) return;
    extents_ = ds_.get_size();
    cache_.resize(boost::extents[extents_[0]][extents_[1]]);
    if (get_current_frame() >= extents_[2]) return;
    if (TypeTraits::HDF5Traits::BatchOperations) {
      HDF5::DataSetIndexD<3> lb(0, 0, get_current_frame());
      HDF5::DataSetIndexD<3> sz = extents_;
      sz[2] = 1;
      typename TypeTraits::Types all = get_as<typename TypeTraits::Types>(ds_.get_block(lb, sz));
      for (unsigned int i = 0; i < extents_[0]; ++i) {
        for (unsigned int j = 0; j < extents_[1]; ++j) {
          cache_[i][j] = all[i * extents_[1] + j];
#ifndef RMF_NDEBUG
          typename TypeTraits::Type read = cache_[i][j],
              fresh = get_as<typename TypeTraits::Type>(ds_.get_value(HDF5::DataSetIndexD<3>(i, j,
                                                                                                 get_current_frame())));
          RMF_INTERNAL_CHECK(read == fresh,
                             "Values don't match");
#endif
        }
      }
    } else {
      for (unsigned int i = 0; i < get_size()[0]; ++i) {
        for (unsigned int j = 0; j < get_size()[1]; ++j) {
          cache_[i][j] = get_as<typename TypeTraits::Type>(ds_.get_value(HDF5::DataSetIndexD<3>(i, j,
                                                                                                    get_current_frame())));
        }
      }
    }
  }
public:
  HDF5DataSetCacheD(): extents_(0, 0, 0),
                       dirty_(false),
                       current_frame_(0) {
  }
  ~HDF5DataSetCacheD() {
    flush();
  }
  void set_current_frame(unsigned int f) {
    flush();
    current_frame_ = f;
      initialize(ds_);
  }
  unsigned int get_current_frame() const {
    return current_frame_;
  }
  void set(HDF5::Group parent, std::string name) {
    dirty_ = false;
    parent_ = parent;
    name_ = name;
    if (parent_.get_has_child(name_)) {
      initialize(parent_.get_child_data_set<typename TypeTraits::HDF5Traits, 3>(name_));
    } else {
      extents_ = HDF5::DataSetIndexD<3>(0, 0, 0);
    }
  }
  void reset() {
    flush();
    ds_ = DS();
    cache_.resize(boost::extents[0][0][0]);
    extents_ = HDF5::DataSetIndexD<3>(0, 0, 0);
  }
  void flush() {
    if (!dirty_) return;
    if (extents_ != ds_.get_size()) {
      ds_.set_size(extents_);
    }
    if (TypeTraits::HDF5Traits::BatchOperations) {
      HDF5::DataSetIndexD<3> sz(get_size()[0], get_size()[1],
                              1);
      typename TypeTraits::Types data(extents_[0] * extents_[1]);
      HDF5::DataSetIndexD<3> lb(0, 0, get_current_frame());
      for (unsigned int i = 0; i < extents_[0]; ++i) {
        for (unsigned int j = 0; j < extents_[1]; ++j) {
          data[i * extents_[1] + j] = cache_[i][j];
        }
      }
      ds_.set_block(lb, sz, get_as<typename TypeTraits::HDF5Traits::Types>(data));
    } else {
      for (unsigned int i = 0; i < get_size()[0]; ++i) {
        for (unsigned int j = 0; j < get_size()[1]; ++j) {
          ds_.set_value(HDF5::DataSetIndexD<3>(i, j, get_current_frame()),
                        get_as<typename TypeTraits::HDF5Traits::Type>(cache_[i][j]));
        }
      }
    }
    dirty_ = false;
  }
  void set_size(const HDF5::DataSetIndexD<3> &ijk) {
    RMF_INTERNAL_CHECK(!name_.empty(),
                       "Name never set");
    if (ds_ == DS()) {
      HDF5::DataSetCreationPropertiesD<typename TypeTraits::HDF5Traits, 3> props;
      props.set_chunk_size(HDF5::DataSetIndexD<3>(256, 4, 1));
      props.set_compression(HDF5::GZIP_COMPRESSION);
      ds_ = parent_.add_child_data_set<typename TypeTraits::HDF5Traits, 3>(name_, props);
    }
    if (ijk[0] > cache_.shape()[0]
        || ijk[1] > cache_.shape()[1]) {
      cache_.resize(boost::extents[2 * ijk[0]][2 * ijk[1]]);
      // resize cache and fill
      for (unsigned int i = extents_[0]; i < cache_.shape()[0]; ++i) {
        for (unsigned int j = 0; j < cache_.shape()[1]; ++j) {
          cache_[i][j] = TypeTraits::get_null_value();
        }
      }
      for (unsigned int i = 0; i < extents_[0]; ++i) {
        for (unsigned int j = extents_[1]; j < cache_.shape()[1]; ++j) {
          cache_[i][j] = TypeTraits::get_null_value();
        }
      }
    }
    dirty_ = true;
    extents_ = ijk;
  }
  void set_value(const HDF5::DataSetIndexD<3> &ijk,
                 typename TypeTraits::Type  value) {
    RMF_INTERNAL_CHECK(ijk[2] == get_current_frame(),
                       "Frames don't match");
    cache_[ijk[0]][ijk[1]] = value;
    dirty_ = true;
  }
  typename TypeTraits::Types get_row(const HDF5::DataSetIndexD<2> &ij) {
    flush();
    return get_as<typename TypeTraits::Types>(ds_.get_row(ij));
  }
  typename TypeTraits::Type get_value(const HDF5::DataSetIndexD<3> &ijk) const {
    RMF_INTERNAL_CHECK(ijk[2] == get_current_frame(),
                       "Frames don't match");
    typename TypeTraits::Type ret = cache_[ijk[0]][ijk[1]];
    return ret;
  }
  HDF5::DataSetIndexD<3> get_size() const {
    return extents_;
  }
};

}   // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H */
