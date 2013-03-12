/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H

#include <RMF/config.h>
#include <RMF/HDF5/DataSetD.h>
#include "HDF5DataSetCacheD.h"
#include <boost/multi_array.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace hdf5_backend {

template <class TypeTraits>
class HDF5DataSetCacheD<TypeTraits, 1>/*: public boost::noncopyable*/ {
  typedef HDF5::DataSetD<typename TypeTraits::HDF5Traits, 1> DS;
  typedef typename TypeTraits::Types array_type;
  array_type cache_;
  int dirty_begin_, dirty_end_;
  DS ds_;
  HDF5::Group parent_;
  std::string name_;
  void initialize(DS ds) {
    RMF_USAGE_CHECK(dirty_begin_ >= dirty_end_,
                    "Trying to set one that is already set");
    ds_ = ds;
    HDF5::DataSetIndexD<1> sz = ds_.get_size();
    HDF5::DataSetIndexD<1> lb(0);
    if (sz != lb) {
      if (TypeTraits::HDF5Traits::BatchOperations) {
        typename TypeTraits::Types all = ds_.get_block(lb, sz);
        std::swap(cache_, all);
      } else {
        cache_.resize(sz[0]);
        for (unsigned int i = 0; i < cache_.size(); ++i) {
          cache_[i] = ds_.get_value(HDF5::DataSetIndexD<1>(i));
        }
      }
    }
    dirty_begin_ = sz[0];
    dirty_end_ = 0;
  }
public:
  HDF5DataSetCacheD(): dirty_begin_(-1),
                       dirty_end_(-1) {
  }
  ~HDF5DataSetCacheD() {
    flush();
  }
  void set(HDF5::Group parent, std::string name) {
    parent_ = parent;
    name_ = name;
    if (parent_.get_has_child(name_)) {
      initialize(parent_.get_child_data_set<typename TypeTraits::HDF5Traits, 1>(name_));
    } else {
      RMF_TRACE(get_logger(), "No data set " << name);
      dirty_begin_ = 0;
      dirty_end_ = 0;
    }
  }
  void reset() {
    flush();
    ds_ = DS();
    cache_.clear();
  }
  void flush() {
    if (dirty_begin_ >= dirty_end_) return;
    if (ds_.get_size()[0] != cache_.size()) {
      ds_.set_size(HDF5::DataSetIndexD<1>(cache_.size()));
    }
    if (TypeTraits::HDF5Traits::BatchOperations) {
      HDF5::DataSetIndexD<1> sz(dirty_end_ - dirty_begin_);
      HDF5::DataSetIndexD<1> lb(dirty_begin_);
      typename TypeTraits::Types to_copy(cache_.begin() + dirty_begin_,
                                         cache_.end() + dirty_end_);
      ds_.set_block(lb, sz, to_copy);
    } else {
      for ( int i = dirty_begin_; i < dirty_end_; ++i) {
        ds_.set_value(HDF5::DataSetIndexD<1>(i), cache_[i]);
      }
    }
    dirty_begin_ = ds_.get_size()[0];
    dirty_end_ = -1;
  }
  void set_size(const HDF5::DataSetIndexD<1> &ijk) {
    RMF_INTERNAL_CHECK(!name_.empty(),
                       "Name never set");
    if (ds_ == DS()) {
      HDF5::DataSetCreationPropertiesD<typename TypeTraits::HDF5Traits, 1> props;
      props.set_chunk_size(HDF5::DataSetIndexD<1>(256));
      props.set_compression(HDF5::GZIP_COMPRESSION);
      ds_ = parent_.add_child_data_set<typename TypeTraits::HDF5Traits, 1>(name_, props);
    }
    dirty_begin_ = std::min<int>(dirty_begin_, cache_.size());
    dirty_end_ = ijk[0];
    cache_.resize(ijk[0], TypeTraits::get_null_value());

  }
  void set_value(const HDF5::DataSetIndexD<1> &ijk,
                 typename TypeTraits::Type  value) {
    RMF_USAGE_CHECK(ijk[0] < cache_.size(),
                    "Not big enough");
    cache_[ijk[0]] = value;
    dirty_begin_ = std::min<int>(dirty_begin_, ijk[0]);
    dirty_end_ = std::max<int>(dirty_end_, ijk[0] + 1);
  }
  typename TypeTraits::Type get_value(const HDF5::DataSetIndexD<1> &ijk) const {
    return cache_[ijk[0]];
  }
  HDF5::DataSetIndexD<1> get_size() const {
    return HDF5::DataSetIndexD<1>(cache_.size());
  }
};

}   // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H */
