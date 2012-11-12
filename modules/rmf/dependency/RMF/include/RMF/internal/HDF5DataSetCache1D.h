/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H

#include <RMF/config.h>
#include "../HDF5DataSetD.h"
#include "HDF5DataSetCacheD.h"
#include <boost/multi_array.hpp>
namespace RMF {


  namespace internal {

    template <class TypeTraits>
    class HDF5DataSetCacheD<TypeTraits, 1>/*: public boost::noncopyable*/ {
      typedef HDF5DataSetD<TypeTraits, 1> DS;
      typedef typename TypeTraits::Types array_type;
      array_type cache_;
      bool dirty_;
      DS ds_;
      HDF5Group parent_;
      std::string name_;
      void initialize(DS ds) {
        RMF_USAGE_CHECK(!dirty_, "Trying to set one that is already set");
        ds_=ds;
        HDF5DataSetIndexD<1> sz=ds_.get_size();
        HDF5DataSetIndexD<1> lb(0);
        if (sz != lb) {
          if (TypeTraits::BatchOperations) {
            typename TypeTraits::Types all= ds_.get_block(lb, sz);
            std::swap(cache_, all);
          } else {
            cache_.resize(sz[0]);
            for (unsigned int i=0; i< cache_.size(); ++i) {
              cache_[i]= ds_.get_value(HDF5DataSetIndexD<1>(i));
            }
          }
        }
      }
    public:
      HDF5DataSetCacheD(): dirty_(false) {}
      ~HDF5DataSetCacheD() {
        flush();
      }
      void set(HDF5Group parent, std::string name) {
        dirty_=false;
        parent_=parent;
        name_=name;
        if (parent_.get_has_child(name_)) {
          initialize(parent_.get_child_data_set<TypeTraits, 1>(name_));
        }
      }
      void reset() {
        flush();
        ds_=DS();
        cache_.clear();
      }
      void flush() {
        if (!dirty_) return;
        ds_.set_size(HDF5DataSetIndexD<1>(cache_.size()));
        if (TypeTraits::BatchOperations) {
          HDF5DataSetIndexD<1> sz=ds_.get_size();
          HDF5DataSetIndexD<1> lb(0);
          ds_.set_block(lb, sz, cache_);
        } else {
          for (unsigned int i=0; i< cache_.size(); ++i) {
            ds_.set_value(HDF5DataSetIndexD<1>(i), cache_[i]);
          }
        }
        dirty_=false;
      }
      void set_size(const HDF5DataSetIndexD<1> &ijk) {
        RMF_INTERNAL_CHECK(!name_.empty(),
                           "Name never set");
        if (ds_== DS()) {
          HDF5DataSetCreationPropertiesD<TypeTraits, 1> props;
          props.set_chunk_size(HDF5DataSetIndexD<1>(256));
          props.set_compression(GZIP_COMPRESSION);
          ds_= parent_.add_child_data_set<TypeTraits, 1>(name_, props);
        }
        cache_.resize(ijk[0], TypeTraits::get_null_value());
        dirty_=true;
      }
      void set_value(const HDF5DataSetIndexD<1> &ijk,
                     typename TypeTraits::Type value) {
        RMF_USAGE_CHECK(ijk[0] < cache_.size(),
                        "Not big enough");
        cache_[ijk[0]]=value;
        dirty_=true;
      }
      typename TypeTraits::Type get_value(const HDF5DataSetIndexD<1> &ijk) const {
        return cache_[ijk[0]];
      }
      HDF5DataSetIndexD<1> get_size() const {
        return HDF5DataSetIndexD<1>(cache_.size());
      }
    };

  } // namespace internal
} /* namespace RMF */


#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_1D_H */
