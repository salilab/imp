/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_HDF_5SHARED_DATA_H
#define IMPLIBRMF_INTERNAL_HDF_5SHARED_DATA_H

#include "../RMF_config.h"
#include "SharedData.h"
#include "../HDF5Group.h"
#include "../HDF5File.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <hdf5.h>
#include <algorithm>

#include <boost/shared_ptr.hpp>

namespace RMF {


  namespace internal {

#define IMP_RMF_HDF5_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues)              \
    DataDataSetCache<Ucname##Traits, 2> lcname##_data_sets_;            \
    DataDataSetCache<Ucname##Traits, 3> per_frame_##lcname##_data_sets_; \
    HDF5DataSetD<Ucname##Traits, 2>&                                    \
    get_data_set_i(Ucname##Traits,                                      \
                   int kc, int arity,                                   \
                   bool create_if_needed) const {                       \
      return lcname##_data_sets_.get(file_, kc,                         \
                                     get_category_name(arity, kc),arity, \
                                     create_if_needed);                 \
    }                                                                   \
    HDF5DataSetD<Ucname##Traits, 3>&                                    \
    get_per_frame_data_set_i(Ucname##Traits,                            \
                             int kc, int arity,                         \
                             bool create_if_needed) const {             \
      return per_frame_##lcname##_data_sets_.get(file_, kc,             \
                                                 get_category_name(arity, kc), \
                                                 arity,                 \
                                                 create_if_needed);     \
    }



#define IMP_RMF_HDF5_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, Arity)      \
    Ucname##Traits::Type get_value(unsigned int node,                   \
                                   Key<Ucname##Traits,Arity> k,         \
                                   unsigned int frame) const {          \
      return get_value_impl(node, k, frame);                            \
    }                                                                   \
    Ucname##Traits::Types get_all_value(unsigned int node,              \
                                        Key<Ucname##Traits,Arity> k) const { \
      return get_all_values_impl(node, k);                              \
    }                                                                   \
    void set_value(unsigned int node,                                   \
                   Key<Ucname##Traits, Arity> k,                        \
                        Ucname##Traits::Type v, unsigned int frame) {   \
      return set_value_impl(node, k, v, frame);                         \
    }                                                                   \
    virtual void set_values(unsigned int node,                          \
                            const vector<Key<Ucname##Traits, Arity> > &k, \
                            const Ucname##Traits::Types v,              \
                            unsigned int frame) {                       \
      return set_values_impl(node, k, v, frame);                        \
    }                                                                   \
    Key<Ucname##Traits, Arity> add_##lcname##_key_##Arity(int category_id, \
                                                          std::string name, \
                                                          bool per_frame) { \
      return add_key_impl<Ucname##Traits, Arity>(category_id,           \
                                                 name, per_frame);      \
    }                                                                   \
    unsigned int                                                        \
    get_number_of_##lcname##_keys_##Arity(int category_id,              \
                                          bool per_frame) const {       \
      return get_number_of_keys_impl<Ucname##Traits, Arity>(category_id, \
                                                            per_frame); \
    }                                                                   \
    std::string get_name(Key<Ucname##Traits, Arity> k) const {          \
      return get_name_impl(k);                                          \
    }

#define IMP_RMF_HDF5_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                            PassValues, ReturnValues)               \
    IMP_RMF_HDF5_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 1);           \
    IMP_RMF_HDF5_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 2);           \
    IMP_RMF_HDF5_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 3);           \
    IMP_RMF_HDF5_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 4)


    class RMFEXPORT HDF5SharedData: public SharedData {
      // indexed first by per frame, then by
      // TypeInfo::get_index() then by ID
      // then by key.get_index()
      std::string file_name_;
      mutable HDF5Group file_;
      HDF5DataSetD<StringTraits, 1> node_names_;
      mutable HDF5DataSetD<StringTraits, 1> fame_names_;
      boost::array<HDF5DataSetD<StringTraits, 1>, 4> category_names_;
      boost::array<Strings, 4> category_names_cache_;
      boost::array<HDF5DataSetD<IndexTraits, 2>, 4> node_data_;
      boost::array<Ints,4> free_ids_;
      unsigned int frames_hint_;

      // caches
      typedef vector< vector<int> > IndexCache;
      mutable boost::array<IndexCache, 4> index_cache_;

      template <class TypeTraits, unsigned int D>
        class DataDataSetCache {
        typedef HDF5DataSetD<TypeTraits, D> DS;
        mutable vector<DS > cache_;
        mutable HDF5DataSetD<TypeTraits, D> null_;
      public:
        HDF5DataSetD<TypeTraits,D>& get(HDF5Group file,
                                        unsigned int kc,
                                        std::string kcname,
                                        int arity,
                                        bool create_if_needed) const {
          bool found=true;
          if (cache_.size() <= kc) {
            found=false;
          } else if (cache_[kc] ==null_) {
            found=false;
          }
          if (!found) {
            std::string nm=get_data_data_set_name(kcname, arity,
                                                  TypeTraits::get_name(),
                                                  D==3);
            if (!file.get_has_child(nm)) {
              if (!create_if_needed) {
                return null_;
              } else {
                HDF5DataSetCreationPropertiesD<TypeTraits, D> props;
                if (D==3) {
                  props.set_chunk_size(HDF5DataSetIndexD<D>(256, 4, 100));
                } else if (D==2) {
                  props.set_chunk_size(HDF5DataSetIndexD<D>(256, 4));
                } else {
                  IMP_RMF_INTERNAL_CHECK(false,
                                         "Where did this dimension come from");
                }
                props.set_compression(GZIP_COMPRESSION);
                file.add_child_data_set<TypeTraits, D>(nm, props);
              }
            }
            HDF5DataSetD<TypeTraits, D> ds
              = file.get_child_data_set<TypeTraits, D>(nm);
            cache_.resize(std::max(cache_.size(),
                                   static_cast<size_t>(kc+1)));
            cache_[kc]=ds;
          }
          return cache_[kc];
        }
      };
      class KeyNameDataSetCache {
        // category, type, per_frame
        typedef HDF5DataSetD<StringTraits, 1> DS;
        typedef std::pair<DS, DS> Pair;
        mutable vector<vector< Pair > > cache_;
        mutable HDF5DataSetD<StringTraits, 1> null_;
      public:
        HDF5DataSetD<StringTraits, 1>& get(HDF5Group file,
                                           unsigned int kc,
                                           std::string kcname,
                                           int Arity,
                                           unsigned int type_index,
                                           std::string type_name,
                                           bool per_frame,
                                           bool create_if_needed) const {
          bool found=true;
          if (cache_.size() <= kc) {
            found=false;
          } else if (cache_[kc].size() <= type_index) {
            found=false;
          } else if ((per_frame
                      && cache_[kc][type_index].first
                      ==null_)
                     || (!per_frame
                         && cache_[kc][type_index].second
                         ==null_)) {
            found=false;
          }
          if (!found) {
            std::string nm= get_key_list_data_set_name(kcname,
                                                       Arity,
                                                       type_name,
                                                       per_frame);
            if (!file.get_has_child(nm)) {
              if (!create_if_needed) {
                return null_;
              } else {
                HDF5DataSetCreationPropertiesD<StringTraits, 1> props;
                props.set_compression(GZIP_COMPRESSION);
                HDF5DataSetD<StringTraits, 1> ds
                    = file.add_child_data_set<StringTraits, 1>
                    (nm, props);
              }
            }
            HDF5DataSetD<StringTraits, 1> ds
                = file.get_child_data_set<StringTraits, 1>(nm);
            cache_.resize(std::max(cache_.size(),
                                   static_cast<size_t>(kc+1)));
            cache_[kc]
              .resize(std::max(cache_[kc].size(),
                               static_cast<size_t>(type_index+1)));
            if (per_frame) {
              cache_[kc][type_index].first=ds;
            } else {
              cache_[kc][type_index].second=ds;
            }
          }
          if (per_frame) {
            return cache_[kc][type_index].first;
          } else {
            return cache_[kc][type_index].second;
          }
        }
      };
      mutable Ints max_cache_;
      mutable set<std::string> known_data_sets_;
      boost::array<KeyNameDataSetCache,4> key_name_data_sets_;
      IMP_RMF_FOREACH_TYPE(IMP_RMF_HDF5_SHARED_DATA_TYPE);

      template <class TypeTraits>
        HDF5DataSetD<StringTraits, 1>&
        get_key_list_data_set(unsigned int kc,
                              int arity,
                              bool per_frame,
                              bool create_if_needed) const {
        return key_name_data_sets_[arity-1].get(file_, kc,
                                                get_category_name(arity, kc),
                                                arity,
                                                TypeTraits::get_index(),
                                                TypeTraits::get_name(),
                                                per_frame, create_if_needed);
      }

      template <class TypeTraits>
        HDF5DataSetD<TypeTraits, 2>&
        get_data_data_set(unsigned int kc, int arity,
                          bool create_if_needed) const {
        return get_data_set_i(TypeTraits(), kc, arity, create_if_needed);
      }
      template <class TypeTraits>
        HDF5DataSetD<TypeTraits, 3>&
        get_per_frame_data_data_set(unsigned int kc, int arity,
                                    bool create_if_needed) const {
        return get_per_frame_data_set_i(TypeTraits(), kc, arity,
                                        create_if_needed);
      }

      enum Indexes {TYPE=0, CHILD=1, SIBLING=2, FIRST_KEY=3};

      unsigned int get_index(int Arity, unsigned int kc) const {
        switch (Arity) {
        case 1:
          return kc+FIRST_KEY;
        default:
          return 1+Arity+kc;
        }
      }
      void check_node(unsigned int node) const;
      template <int Arity>
        unsigned int get_column_maximum(unsigned int kc) const {
        if (max_cache_.size() > kc
            && max_cache_[kc]>-2) {
          return max_cache_[kc];
        }
        HDF5DataSetIndexD<2> sz= node_data_[Arity-1].get_size();
        int mx=-1;
        int index= get_index(Arity, kc);
        for (unsigned int i=0; i< sz[0]; ++i) {
          mx= std::max(mx,
                       node_data_[Arity-1]
                       .get_value(HDF5DataSetIndexD<2>(i, index)));
        }
        max_cache_.resize(std::max(max_cache_.size(),
                                   static_cast<size_t>(kc+1)), -2);
        max_cache_[kc]=mx;
        return mx;
      }

      template <class TypeTraits, int Arity>
        typename TypeTraits::Types get_all_values_impl(unsigned int node,
                                                      Key<TypeTraits,Arity> k)
        const {
        IMP_RMF_BEGIN_FILE
          IMP_RMF_USAGE_CHECK(k.get_is_per_frame(),
                      "Using get_all_values on a key that is not per_frame.");
        unsigned int kc=k.get_category().get_index();
        int vi=get_index_from_cache<Arity>(node, kc);
        if (IndexTraits::get_is_null_value(vi)) {
          IMP_RMF_BEGIN_OPERATION
          int index= get_index(Arity, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[Arity-1].get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            return typename TypeTraits::Types();
          } else {
            vi=node_data_[Arity-1].get_value(HDF5DataSetIndexD<2>(node, index));
          }
          if (IndexTraits::get_is_null_value(vi)) {
            return typename TypeTraits::Types();
          } else {
            add_index_to_cache<Arity>(node, kc, vi);
          }
          IMP_RMF_END_OPERATION("getting value index");
        }
        {
          IMP_RMF_BEGIN_OPERATION
            HDF5DataSetD<TypeTraits, 3> &ds
            = get_per_frame_data_data_set<TypeTraits>(kc,
                                                      k.get_arity(),
                                                      false);
          if (!ds) return typename TypeTraits::Types();
          HDF5DataSetIndexD<3> sz= ds.get_size();
          if (static_cast<hsize_t>(vi) >= sz[0]
              || static_cast<hsize_t>(k.get_index()) >= sz[1]) {
            return typename TypeTraits::Types();
          } else {
            return ds.get_row(HDF5DataSetIndexD<2>(vi, k.get_index()));
          }
          IMP_RMF_END_OPERATION("fetching data from per frame data set");
        }
        IMP_RMF_END_FILE(get_file_name());
      }

      template <class TypeTraits, int Arity>
        typename TypeTraits::Type get_value_impl(unsigned int node,
                                                 Key<TypeTraits,Arity> k,
                                                 unsigned int frame) const {
        IMP_RMF_BEGIN_FILE
        unsigned int kc=k.get_category().get_index();
        bool per_frame= k.get_is_per_frame();
        int vi=get_index_from_cache<Arity>(node, kc);
        if (IndexTraits::get_is_null_value(vi)) {
          IMP_RMF_BEGIN_OPERATION
          int index= get_index(Arity, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[Arity-1].get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            return TypeTraits::get_null_value();
          } else {
            vi=node_data_[Arity-1].get_value(HDF5DataSetIndexD<2>(node, index));
          }
          if (IndexTraits::get_is_null_value(vi)) {
            return TypeTraits::get_null_value();
          } else {
            add_index_to_cache<Arity>(node, kc, vi);
          }
          IMP_RMF_END_OPERATION("getting value index");
        }
        {
          if (per_frame) {
            IMP_RMF_BEGIN_OPERATION
            HDF5DataSetD<TypeTraits, 3> &ds
              = get_per_frame_data_data_set<TypeTraits>(kc,
                                                        k.get_arity(),
                                                        false);
            if (!ds) return TypeTraits::get_null_value();
            HDF5DataSetIndexD<3> sz= ds.get_size();
            if (static_cast<hsize_t>(vi) >= sz[0]
                || static_cast<hsize_t>(k.get_index()) >= sz[1]
                || (frame >= static_cast<unsigned int>(sz[2]))) {
              return TypeTraits::get_null_value();
            } else {
              return ds.get_value(HDF5DataSetIndexD<3>(vi, k.get_index(),
                                                       frame));
            }
            IMP_RMF_END_OPERATION("fetching data from per frame data set")
          } else {
            IMP_RMF_BEGIN_OPERATION
            HDF5DataSetD<TypeTraits,2> &ds
              = get_data_data_set<TypeTraits>(kc,
                                              k.get_arity(),
                                              false);
            if (!ds) return TypeTraits::get_null_value();
            HDF5DataSetIndexD<2> sz= ds.get_size();
            if (static_cast<hsize_t>(vi) >= sz[0]
                || static_cast<hsize_t>(k.get_index()) >= sz[1]) {
              return TypeTraits::get_null_value();
            } else {
              return ds.get_value(HDF5DataSetIndexD<2>(vi, k.get_index()));
            }
            IMP_RMF_END_OPERATION("fetching data from data set");
          }
        }
        IMP_RMF_END_FILE(get_file_name());
      }
      template <class TypeTraits, int Arity>
        unsigned int get_number_of_frames(Key<TypeTraits, Arity> k) const {
        unsigned int kc= k.get_category().get_index();
        if (!get_is_per_frame(k)) {
          IMP_RMF_THROW(get_error_message("Attribue ", get_name(k),
                                          " does not have frames."),
                        UsageException);
          IMP_RMF_NO_RETURN(int);
        } else {
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     k.get_arity(),
                                                     false);
          if (!ds) {
            return 0;
            /*IMP_RMF_THROW("Attribute " << get_name(k)
                          << " does not have any data.",
                          UsageException);*/
          }
          HDF5DataSetIndexD<3> sz= ds.get_size();
          return sz[2];
        }
      }
      template <int Arity>
          int get_index_from_cache(unsigned int node, unsigned int kc) const {
        if (index_cache_[Arity-1].size() <= node) return -1;
        else if (index_cache_[Arity-1][node].size() <= kc) return -1;
        return index_cache_[Arity-1][node][kc];
      }
      template <int Arity>
          void add_index_to_cache(unsigned int node, unsigned int kc,
                                    int index) const {
        if (index_cache_[Arity-1].size() <= node) {
          index_cache_[Arity-1].resize(node+1, vector<int>());
        }
        if (index_cache_[Arity-1][node].size() <= kc) {
          index_cache_[Arity-1][node].resize(kc+1, -1);
        }
        index_cache_[Arity-1][node][kc] =index;
      }

      template <int Arity>
          int get_index_set(unsigned int node, unsigned int kc) {
        int vi=get_index_from_cache<Arity>(node, kc);
        if (vi==-1) {
          IMP_RMF_BEGIN_OPERATION;
          unsigned int index= get_index(Arity, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[Arity-1].get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          if (nsz[1] <=index) {
            HDF5DataSetIndexD<2> newsz= nsz;
            newsz[1]= index+1;
            node_data_[Arity-1].set_size(newsz);
          }
          // now it is big enough
          // make sure the target table is there
          /*if (!get_has_data_set(nm)) {
            file_.add_data_set<TypeTraits>(nm, (per_frame?3:2));
            }*/
          // now we have the index and the data set is there
          vi= node_data_[Arity-1].get_value(HDF5DataSetIndexD<2>(node, index));
          if (IndexTraits::get_is_null_value(vi)) {
            vi= get_column_maximum<Arity>(kc)+1;
            node_data_[Arity-1].set_value(HDF5DataSetIndexD<2>(node,
                                                               index), vi);
            max_cache_[kc]=vi;
          }
          add_index_to_cache<Arity>(node, kc, vi);
          IMP_RMF_END_OPERATION("figuring out where to store value");
        }
        return vi;
      }

      template <class TypeTraits, int Arity>
          void make_fit( HDF5DataSetD<TypeTraits, 3> &ds,
                         int vi,
                         Key<TypeTraits, Arity> k, unsigned int frame) {
        HDF5DataSetIndexD<3> sz= ds.get_size();
        bool delta=false;
        if (sz[0] <= static_cast<hsize_t>(vi)) {
          sz[0] = vi+1;
          delta=true;
        }
        if (sz[1] <= static_cast<hsize_t>(k.get_index())) {
          sz[1]= k.get_index()+1;
          delta=true;
        }
        if (static_cast<unsigned int>(sz[2]) <= frame) {
          sz[2] =std::max(frame+1, frames_hint_);
            delta=true;
        }
        if (delta) {
          ds.set_size(sz);
        }
      }
      template <class TypeTraits, int Arity>
          void make_fit( HDF5DataSetD<TypeTraits, 2> &ds,
                         int vi,
                         Key<TypeTraits, Arity> k) {
        HDF5DataSetIndexD<2> sz= ds.get_size();
        bool delta=false;
        if (sz[0] <= static_cast<hsize_t>(vi)) {
          sz[0] = vi+1;
          delta=true;
        }
        if (sz[1] <= static_cast<hsize_t>(k.get_index())) {
          sz[1]= k.get_index()+1;
          delta=true;
        }
        if (delta) {
          ds.set_size(sz);
        }
      }
      template <class TypeTraits, int Arity>
        void set_value_impl(unsigned int node, Key<TypeTraits, Arity> k,
                       typename TypeTraits::Type v, unsigned int frame) {
        IMP_RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(v),
                            "Cannot write sentry value to an RMF file.");
        unsigned int kc= k.get_category().get_index();
        bool per_frame= get_is_per_frame(k);
        int vi=get_index_set<Arity>(node, kc);
        if (per_frame) {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     k.get_arity(), true);
          make_fit(ds, vi, k, frame);
          ds.set_value(HDF5DataSetIndexD<3>(vi, k.get_index(), frame), v);
          IMP_RMF_END_OPERATION("storing per frame value");
        } else {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 2> &ds
            =get_data_data_set<TypeTraits>(kc, k.get_arity(),
                                           true);
          make_fit(ds, vi, k);
          ds.set_value(HDF5DataSetIndexD<2>(vi, k.get_index()), v);
          IMP_RMF_END_OPERATION("storing single value")
        }
        /*IMP_RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                               "Stored " << v << " but got "
                               << get_value(node, k, frame));*/
      }


      template <class TypeTraits, int Arity>
          void set_values_impl(unsigned int node,
                               const vector<Key<TypeTraits, Arity> > &k,
                               const typename TypeTraits::Types& v,
                               unsigned int frame) {
        IMP_RMF_USAGE_CHECK(v.size()== k.size(),
                            "Size of values and keys don't match.");
        if (k.empty()) return;
        unsigned int kc= k[0].get_category().get_index();
        bool per_frame= get_is_per_frame(k[0]);
        int vi=get_index_set<Arity>(node, kc);
        if (per_frame) {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     k.front().get_arity(),
                                                     true);
          make_fit(ds, vi, k.back(), frame);
          ds.set_block(HDF5DataSetIndexD<3>(vi, k.front().get_index(), frame),
                       HDF5DataSetIndexD<3>(1, k.size(), 1),
                       v);
          IMP_RMF_END_OPERATION("storing per frame value");
        } else {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 2> &ds
              =get_data_data_set<TypeTraits>(kc, k.front().get_arity(),
                                           true);
          make_fit(ds, vi, k.back());
          ds.set_block(HDF5DataSetIndexD<2>(vi, k.front().get_index()),
                       HDF5DataSetIndexD<2>(1, k.size()),
                       v);
          IMP_RMF_END_OPERATION("storing single value")
        }
        /*IMP_RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                               "Stored " << v << " but got "
                               << get_value(node, k, frame));*/
      }

      template <class TypeTraits, int Arity>
        Key<TypeTraits, Arity> add_key_impl(int category_id,
                                       std::string name, bool per_frame) {
        audit_key_name(name);
        // check that it is unique
        IMP_RMF_BEGIN_OPERATION;
        {
          HDF5DataSetD<StringTraits, 1> &nameds
            = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                                per_frame, true);
          unsigned int sz= nameds.get_size()[0];
          HDF5DataSetIndexD<1> index;
          for (unsigned int i=0; i< sz; ++i) {
            index[0]=i;
            IMP_RMF_USAGE_CHECK(nameds.get_value(index) != name,
                                get_error_message("Attribute name ", name,
                                             " already taken for that type."));
          }
        }
        IMP_RMF_END_OPERATION("checking that key is unique");
        IMP_RMF_BEGIN_OPERATION;
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                              per_frame,
                                              true);
        HDF5DataSetIndexD<1> sz= nameds.get_size();
        int ret_index= sz[0];
        ++sz[0];
        nameds.set_size(sz);
        --sz[0];
        nameds.set_value(sz, name);
        return Key<TypeTraits, Arity>(category_id, ret_index, per_frame);
        IMP_RMF_END_OPERATION("appending key to list")
      }

      // create the data sets and add rows to the table
      template <class TypeTraits, int Arity>
        unsigned int
        get_number_of_keys_impl(int category_id, bool per_frame) const {
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                              per_frame, false);
        if (!nameds) return 0;
        HDF5DataSetIndexD<1> sz= nameds.get_size();
        return sz[0];
      }

      template <class TypeTraits, int Arity>
        std::string get_name_impl(Key<TypeTraits, Arity> k) const {
        unsigned int kc=k.get_category().get_index();
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(kc, k.get_arity(),
                                              k.get_is_per_frame(),
                                              false);
        IMP_RMF_USAGE_CHECK(nameds, "No keys of the desired category found");
        HDF5DataSetIndexD<1> index(k.get_index());
        return nameds.get_value(index);
      }
      template <class TypeTraits, int Arity>
        Key<TypeTraits, Arity> get_key_impl(int category_id,
                                            std::string name,
                                            bool per_frame) const {
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                              per_frame,
                                              false);
        if (!nameds) {
          return Key<TypeTraits, Arity>();
        }
        HDF5DataSetIndexD<1> size= nameds.get_size();
        for (unsigned int j=0; j< size[0]; ++j) {
          HDF5DataSetIndexD<1> index(j);
          std::string cur=nameds.get_value(index);
          if (cur== name) {
            return Key<TypeTraits, Arity>(category_id, j, per_frame);
          }
        }
        return Key<TypeTraits, Arity>();
      }

      void initialize_categories(int i, bool create);
      void initialize_keys(int i);
      void initialize_free_nodes();

      int get_first_child(unsigned int node) const;
      int get_sibling(unsigned int node) const;
      void set_first_child(unsigned int node, int child);
      void set_sibling(unsigned int node, int sibling);
      int add_node(std::string name, unsigned int type);
      void check_set(int arity, unsigned int index) const;
      void close_things();

      // opens the file in file_name_
      // @param create - whether to create the file or just open it
      void open_things(bool create);
    public:
      IMP_RMF_FOREACH_TYPE(IMP_RMF_HDF5_SHARED_TYPE);

      HDF5Group get_group() const {
        return file_;
      }
      void flush() const {
        IMP_HDF5_CALL(H5Fflush(file_.get_handle(), H5F_SCOPE_GLOBAL));
        //SharedData::validate();
      }
      std::string get_file_name() const {
        return file_.get_file().get_name();
      }

      /**
         constructs HDF5SharedData for the RMF file g, either creating
         or opening the file according to the value of create.

         @param g - path to file
         @param create - whether to create the file or just open it
         @exception RMF::IOException if couldn't create / open file
                    or bad file format
      */
      HDF5SharedData(std::string g, bool create);
      ~HDF5SharedData();
      std::string get_name(unsigned int node) const;
      unsigned int get_type(unsigned int Arity, unsigned int node) const;
      int add_child(int node, std::string name, int t);
      Ints get_children(int node) const;
      void save_frames_hint(int i) {
        frames_hint_=i;
      }
      unsigned int get_number_of_frames() const;
      unsigned int get_number_of_sets(int arity) const;
      unsigned int add_set( RMF::Indexes nis, int t);
      unsigned int get_set_member(int Arity, unsigned int index,
                                    int member_index) const;
      int add_category(int Arity, std::string name);
      unsigned int get_number_of_categories(int Arity) const;
      std::string get_category_name(int Arity, unsigned int kc) const  {
        IMP_RMF_USAGE_CHECK(category_names_cache_.size()
                            >= static_cast<unsigned int>(Arity),
                            get_error_message("No categories with arity ",
                                              Arity));
        IMP_RMF_USAGE_CHECK(category_names_cache_[Arity-1].size() > kc,
                            get_error_message("No such category with arity ",
                                              Arity));
        return category_names_cache_[Arity-1][kc];
      }

      std::string get_description() const;
      void set_description(std::string str);

      void set_frame_name(unsigned int frame, std::string str);
      std::string get_frame_name(unsigned int frame) const;

       bool get_supports_locking() const {return true;}
       bool set_is_locked(bool tf);
       void reload();
    };

  } // namespace internal
} /* namespace RMF */


#endif /* IMPLIBRMF_INTERNAL_HDF_5SHARED_DATA_H */
