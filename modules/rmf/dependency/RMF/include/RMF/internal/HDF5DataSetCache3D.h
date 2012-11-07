/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 3007-3013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H

#include <RMF/config.h>
#include "../HDF5DataSetD.h"
#include "HDF5DataSetCacheD.h"
#include <boost/multi_array.hpp>
namespace RMF {


  namespace internal {

    template <class TypeTraits>
    class HDF5DataSetCacheD<TypeTraits, 3>/*: public boost::noncopyable*/ {
      typedef HDF5DataSetD<TypeTraits, 3> DS;
      typedef boost::multi_array<typename TypeTraits::Type, 2> array_type;
      typedef typename array_type::index index;
      array_type cache_;
      bool dirty_;
      DS ds_;
      HDF5Group parent_;
      std::string name_;
      unsigned int current_frame_;
      unsigned int nframes_;
      void initialize(DS ds) {
        RMF_USAGE_CHECK(!dirty_, "Trying to set a set that is already set");
        ds_=ds;
        if (ds== DS()) return;
        HDF5DataSetIndexD<3> sz=ds_.get_size();
        cache_.resize(boost::extents[sz[0]][sz[1]]);
        nframes_=sz[2];
        if (get_current_frame() >= nframes_) return;
        if (TypeTraits::BatchOperations) {
          HDF5DataSetIndexD<3> lb(0,0,get_current_frame());
          sz[2]= 1;
          typename TypeTraits::Types all= ds_.get_block(lb, sz);
          for (unsigned int i=0; i< sz[0]; ++i) {
            for (unsigned int j=0; j< sz[1]; ++j) {
              cache_[i][j]= all[i*sz[1]+j];
              typename TypeTraits::Type read= cache_[i][j],
                fresh= ds_.get_value(HDF5DataSetIndexD<3>(i,j,
                                                          get_current_frame()));
              RMF_INTERNAL_CHECK(read==fresh,
                                 "Values don't match");
            }
          }
        } else {
          for (unsigned int i=0; i< get_size()[0]; ++i) {
            for (unsigned int j=0; j< get_size()[1]; ++j) {
              cache_[i][j]= ds_.get_value(HDF5DataSetIndexD<3>(i,j,
                                                               get_current_frame()));
            }
          }
        }
      }
    public:
      HDF5DataSetCacheD(): dirty_(false), current_frame_(0), nframes_(0) {}
      ~HDF5DataSetCacheD() {
        flush();
      }
      void set_current_frame(unsigned int f) {
        flush();
        current_frame_=f;
        initialize(ds_);
      }
      unsigned int get_current_frame() const {
        return current_frame_;
      }
      void set(HDF5Group parent, std::string name) {
        dirty_=false;
        parent_=parent;
        name_=name;
        if (parent_.get_has_child(name_)) {
          initialize(parent_.get_child_data_set<TypeTraits, 3>(name_));
        }
      }
      void reset() {
        flush();
        ds_=DS();
        cache_.resize(boost::extents[0][0][0]);
      }
      void flush() {
        if (!dirty_) return;
        if (TypeTraits::BatchOperations) {
          HDF5DataSetIndexD<3> sz(get_size()[0], get_size()[1],
                                  1);
          typename TypeTraits::Types data(sz[0]*sz[1]);
          HDF5DataSetIndexD<3> lb(0,0, get_current_frame());
          for (unsigned int i=0; i< sz[0]; ++i) {
            for (unsigned int j=0; j< sz[1]; ++j) {
              data[i*sz[1]+j] = cache_[i][j];
            }
          }
          ds_.set_block(lb, sz, data);
        } else {
          for (unsigned int i=0; i< get_size()[0]; ++i) {
            for (unsigned int j=0; j< get_size()[1]; ++j) {
              ds_.set_value(HDF5DataSetIndexD<3>(i,j, get_current_frame()),
                            cache_[i][j]);
            }
          }
        }
        dirty_=false;
      }
      void set_size(const HDF5DataSetIndexD<3> &ijk) {
        RMF_INTERNAL_CHECK(!name_.empty(),
                           "Name never set");
        if (ds_== DS()) {
          HDF5DataSetCreationPropertiesD<TypeTraits, 3> props;
          props.set_chunk_size(HDF5DataSetIndexD<3>(256, 4, 8));
          props.set_compression(GZIP_COMPRESSION);
          ds_= parent_.add_child_data_set<TypeTraits, 3>(name_, props);
        } else {
          flush();
        }
        ds_.set_size(ijk);
        initialize(ds_);
      }
      void set_value(const HDF5DataSetIndexD<3> &ijk,
                     typename TypeTraits::Type value) {
        RMF_INTERNAL_CHECK(ijk[2]== get_current_frame(),
                           "Frames don't match");
        cache_[ijk[0]][ijk[1]]=value;
        dirty_=true;
      }
      typename TypeTraits::Types get_row(const HDF5DataSetIndexD<2> &ij) {
        flush();
        return ds_.get_row(ij);
      }
      typename TypeTraits::Type get_value(const HDF5DataSetIndexD<3> &ijk) const {
        RMF_INTERNAL_CHECK(ijk[2]== get_current_frame(),
                           "Frames don't match");
        typename TypeTraits::Type ret= cache_[ijk[0]][ijk[1]];
        return ret;
      }
      HDF5DataSetIndexD<3> get_size() const {
        return HDF5DataSetIndexD<3>(cache_.shape()[0],
                                    cache_.shape()[1], nframes_);
      }
    };

  } // namespace internal
} /* namespace RMF */


#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_3D_H */
