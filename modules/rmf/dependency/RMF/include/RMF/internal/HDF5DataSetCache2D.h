/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H

#include <RMF/config.h>
#include "../HDF5DataSetD.h"
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
      bool dirty_;
      DS ds_;
      HDF5Group parent_;
      std::string name_;
      void initialize(DS ds) {
        RMF_USAGE_CHECK(!dirty_, "Trying to set a set that is already set");
        ds_=ds;
        HDF5DataSetIndexD<2> sz=ds_.get_size();
        cache_.resize(boost::extents[sz[0]][sz[1]]);
        HDF5DataSetIndexD<2> lb(0,0);
        if (sz != lb) {
          if (TypeTraits::BatchOperations) {
            typename TypeTraits::Types all= ds_.get_block(lb, sz);
            for (unsigned int i=0; i< sz[0]; ++i) {
              for (unsigned int j=0; j< sz[1]; ++j) {
                cache_[i][j]= all[i*sz[1]+j];
                RMF_INTERNAL_CHECK(cache_[i][j]
                                   ==ds_.get_value(HDF5DataSetIndexD<2>(i,j)),
                                   "Values don't match");
              }
            }
          } else {
            for (unsigned int i=0; i< sz[0]; ++i) {
              for (unsigned int j=0; j< sz[1]; ++j) {
                cache_[i][j]= ds_.get_value(HDF5DataSetIndexD<2>(i,j));
              }
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
          initialize(parent_.get_child_data_set<TypeTraits, 2>(name_));
        }
      }
      void reset() {
        flush();
        ds_=DS();
        cache_.resize(boost::extents[0][0]);
      }
      void flush() {
        if (!dirty_) return;
        HDF5DataSetIndexD<2> sz=get_size();
        if (TypeTraits::BatchOperations) {
          typename TypeTraits::Types data(sz[0]*sz[1]);
          HDF5DataSetIndexD<2> lb(0,0);
          for (unsigned int i=0; i< sz[0]; ++i) {
            for (unsigned int j=0; j< sz[1]; ++j) {
              data[i*sz[1]+j] = cache_[i][j];
            }
          }
          ds_.set_block(lb, sz, data);
        } else {
          for (unsigned int i=0; i< sz[0]; ++i) {
            for (unsigned int j=0; j< sz[1]; ++j) {
              ds_.set_value(HDF5DataSetIndexD<2>(i,j), cache_[i][j]);
            }
          }
        }
        dirty_=false;
      }
      void set_size(const HDF5DataSetIndexD<2> &ijk) {
        RMF_INTERNAL_CHECK(!name_.empty(),
                           "Name never set");
        if (ds_== DS()) {
          HDF5DataSetCreationPropertiesD<TypeTraits, 2> props;
          props.set_chunk_size(HDF5DataSetIndexD<2>(256, 4));
          props.set_compression(GZIP_COMPRESSION);
          ds_= parent_.add_child_data_set<TypeTraits, 2>(name_, props);
        } else {
          flush();
        }
        ds_.set_size(ijk);
        initialize(ds_);
      }
      void set_value(const HDF5DataSetIndexD<2> &ijk,
                     typename TypeTraits::Type value) {
        cache_[ijk[0]][ijk[1]]=value;
        dirty_=true;
      }
      typename TypeTraits::Type get_value(const HDF5DataSetIndexD<2> &ijk) const {
        typename TypeTraits::Type ret= cache_[ijk[0]][ijk[1]];
        return ret;
      }
      HDF5DataSetIndexD<2> get_size() const {
        return HDF5DataSetIndexD<2>(cache_.shape()[0],
                                    cache_.shape()[1]);
      }
    };

  } // namespace internal
} /* namespace RMF */


#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_2D_H */
