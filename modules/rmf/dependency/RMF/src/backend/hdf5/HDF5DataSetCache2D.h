/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H

#include <RMF/config.h>
#include <RMF/HDF5DataSetD.h>
#include "HDF5DataSetCacheD.h"
#include <boost/multi_array.hpp>
namespace RMF {


namespace internal {

template <class TypeTraits>
class HDF5DataSetCacheD<TypeTraits, 2>/*: public boost::noncopyable*/ {
  typedef HDF5DataSetD<TypeTraits, 2> DS;
  typedef boost::multi_array<typename TypeTraits::Type, 2> array_type;
  typedef typename array_type::index index;
  array_type cache_;
  HDF5DataSetIndexD<2> extents_;
  bool dirty_;
  DS ds_;
  HDF5Group parent_;
  std::string name_;
  void initialize(DS ds) {
    RMF_USAGE_CHECK(!dirty_, "Trying to set a set that is already set");
    ds_ = ds;
    extents_ = ds_.get_size();
    cache_.resize(boost::extents[extents_[0]][extents_[1]]);
    HDF5DataSetIndexD<2> lb(0, 0);
    if (extents_ != lb) {
      if (TypeTraits::BatchOperations) {
        typename TypeTraits::Types all = ds_.get_block(lb, extents_);
        for (unsigned int i = 0; i < extents_[0]; ++i) {
          for (unsigned int j = 0; j < extents_[1]; ++j) {
            cache_[i][j] = all[i * extents_[1] + j];
            RMF_INTERNAL_CHECK(cache_[i][j]
                               == ds_.get_value(HDF5DataSetIndexD<2>(i, j)),
                               "Values don't match");
          }
        }
      } else {
        for (unsigned int i = 0; i < extents_[0]; ++i) {
          for (unsigned int j = 0; j < extents_[1]; ++j) {
            cache_[i][j] = ds_.get_value(HDF5DataSetIndexD<2>(i, j));
          }
        }
      }
    }
  }
public:
  HDF5DataSetCacheD(): extents_(0, 0), dirty_(false) {
  }
  ~HDF5DataSetCacheD() {
    flush();
  }
  void set(HDF5Group parent, std::string name) {
    dirty_ = false;
    parent_ = parent;
    name_ = name;
    if (parent_.get_has_child(name_)) {
      initialize(parent_.get_child_data_set<TypeTraits, 2>(name_));
    } else {
      extents_ = HDF5DataSetIndexD<2>(0, 0);
    }
  }
  void reset() {
    flush();
    ds_ = DS();
    cache_.resize(boost::extents[0][0]);
    extents_ = HDF5DataSetIndexD<2>(0, 0);
  }
  void flush() {
    if (!dirty_) return;
    ds_.set_size(extents_);
    if (TypeTraits::BatchOperations) {
      typename TypeTraits::Types data(extents_[0] * extents_[1]);
      HDF5DataSetIndexD<2> lb(0, 0);
      for (unsigned int i = 0; i < extents_[0]; ++i) {
        for (unsigned int j = 0; j < extents_[1]; ++j) {
          data[i * extents_[1] + j] = cache_[i][j];
        }
      }
      ds_.set_block(lb, extents_, data);
    } else {
      for (unsigned int i = 0; i < extents_[0]; ++i) {
        for (unsigned int j = 0; j < extents_[1]; ++j) {
          ds_.set_value(HDF5DataSetIndexD<2>(i, j), cache_[i][j]);
        }
      }
    }
    dirty_ = false;
  }
  void set_size(const HDF5DataSetIndexD<2> &ijk) {
    RMF_INTERNAL_CHECK(!name_.empty(),
                       "Name never set");
    if (ds_ == DS()) {
      HDF5DataSetCreationPropertiesD<TypeTraits, 2> props;
      props.set_chunk_size(HDF5DataSetIndexD<2>(256, 4));
      props.set_compression(GZIP_COMPRESSION);
      ds_ = parent_.add_child_data_set<TypeTraits, 2>(name_, props);
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
  void set_value(const HDF5DataSetIndexD<2> &ijk,
                 typename TypeTraits::Type  value) {
    cache_[ijk[0]][ijk[1]] = value;
    dirty_ = true;
  }
  typename TypeTraits::Type get_value(const HDF5DataSetIndexD<2> &ijk) const {
    typename TypeTraits::Type ret = cache_[ijk[0]][ijk[1]];
    return ret;
  }
  HDF5DataSetIndexD<2> get_size() const {
    return extents_;
  }
};

}   // namespace internal
} /* namespace RMF */


#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H */
