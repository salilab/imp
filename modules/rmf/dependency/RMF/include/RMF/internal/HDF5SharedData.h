/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__INTERNAL_HDF_5SHARED_DATA_H
#define RMF__INTERNAL_HDF_5SHARED_DATA_H

#include <RMF/config.h>
#include "SharedData.h"
#include "../HDF5Group.h"
#include "../HDF5File.h"
#include "HDF5DataSetCacheD.h"
#include "HDF5DataSetCache2D.h"
#include "HDF5DataSetCache1D.h"
#include "HDF5DataSetCache3D.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/array.hpp>
#include <hdf5.h>
#include <algorithm>
#include <boost/ptr_container/ptr_vector.hpp>


namespace RMF {


  namespace internal {

#define RMF_HDF5_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                                  PassValues, ReturnValues)             \
    DataDataSetCache2D<Ucname##Traits> lcname##_data_sets_;             \
    DataDataSetCache3D<Ucname##Traits> per_frame_##lcname##_data_sets_; \
    HDF5DataSetCacheD<Ucname##Traits, 2>&                               \
    get_data_set_i(Ucname##Traits,                                      \
                   int kc, int arity) const {                           \
      return lcname##_data_sets_.get(file_, kc,                         \
                                     get_category_name(kc),arity);      \
    }                                                                   \
    HDF5DataSetCacheD<Ucname##Traits, 3>&                               \
    get_per_frame_data_set_i(Ucname##Traits,                            \
                             int kc, int arity) const {                 \
      return per_frame_##lcname##_data_sets_.get(file_, kc,             \
                                                 get_category_name(kc), \
                                                 arity);                \
    }



#define RMF_HDF5_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,    \
                                   PassValues, ReturnValues)            \
    Ucname##Traits::Type get_value(unsigned int node,                   \
                                   Key<Ucname##Traits> k) const {       \
      return get_value_impl(node, k, get_current_frame());              \
    }                                                                   \
    Ucname##Traits::Types get_all_values(unsigned int node,             \
                                         Key<Ucname##Traits> k)  {      \
      return get_all_values_impl(node, k);                              \
    }                                                                   \
    void set_value(unsigned int node,                                   \
                   Key<Ucname##Traits> k,                               \
                   Ucname##Traits::Type v) {                            \
      return set_value_impl(node, k, v, get_current_frame());           \
    }                                                                   \
    Key<Ucname##Traits> add_##lcname##_key(int category_id,             \
                                           std::string name,            \
                                           bool per_frame) {            \
      return add_key_impl<Ucname##Traits>(category_id,                  \
                                          name, per_frame);             \
    }                                                                   \
    unsigned int                                                        \
    get_number_of_##lcname##_keys(int category_id,                      \
                                  bool per_frame) const {               \
      return get_number_of_keys_impl<Ucname##Traits>(category_id,       \
                                                     per_frame);        \
    }                                                                   \
    std::string get_name(Key<Ucname##Traits> k) const {                 \
      return get_name_impl(k);                                          \
    }


    class RMFEXPORT HDF5SharedData: public SharedData {
      // indexed first by per frame, then by
      // TypeInfo::get_index() then by ID
      // then by key.get_index()
      mutable HDF5Group file_;
      bool read_only_;
      HDF5DataSetCacheD<StringTraits, 1> node_names_;
      HDF5DataSetCacheD<StringTraits, 1> frame_names_;
      boost::array<HDF5DataSetCacheD<StringTraits, 1>, 4> category_names_;
      boost::array<HDF5DataSetCacheD<IndexTraits, 2>, 4> node_data_;
      boost::array<Ints,4> free_ids_;
      unsigned int frames_hint_;

      int link_category_;
      Key<NodeIDTraits> link_key_;

      // caches
      typedef vector< vector<int> > IndexCache;
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
        DataDataSetCache3D(): frame_(0) {}
        HDF5DataSetCacheD<TypeTraits,3>& get(HDF5Group file,
                                             unsigned int kc,
                                             std::string kcname,
                                             int arity) const {
          bool found=true;
          if (cache_.size() <= kc) {
            found=false;
          } else if (cache_.is_null(kc)) {
            found=false;
          }
          if (!found) {
            std::string nm=get_data_data_set_name(kcname, arity,
                                                  TypeTraits::get_name(),
                                                  true);
            cache_.resize(std::max(cache_.size(),
                                   static_cast<size_t>(kc+1)), NULL);
            cache_.replace(kc, new DS());
            cache_[kc].set_current_frame(frame_);
            cache_[kc].set(file, nm);
          }
          return cache_[kc];
        }
        void set_current_frame(unsigned int f) {
          frame_=f;
          for (unsigned int i=0; i< cache_.size(); ++i) {
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
        HDF5DataSetCacheD<TypeTraits,2>& get(HDF5Group file,
                                             unsigned int kc,
                                             std::string kcname,
                                             int arity) const {
          bool found=true;
          if (cache_.size() <= kc) {
            found=false;
          } else if (cache_.is_null(kc)) {
            found=false;
          }
          if (!found) {
            std::string nm=get_data_data_set_name(kcname, arity,
                                                  TypeTraits::get_name(),
                                                  false);
            cache_.resize(std::max(cache_.size(),
                                   static_cast<size_t>(kc+1)), NULL);
            cache_.replace(kc, new DS());
            cache_[kc].set(file, nm);
          }
          return cache_[kc];
        }
      };
      class KeyNameDataSetCache {
        // category, type, per_frame
        typedef HDF5DataSetCacheD<StringTraits, 1> DS;
        typedef boost::ptr_vector<boost::nullable<DS> > PVDS;
        typedef boost::array<PVDS, 2> Pair;
        mutable vector<Pair > cache_;
      public:
        HDF5DataSetCacheD<StringTraits, 1>& get(HDF5Group file,
                                                unsigned int kc,
                                                std::string kcname,
                                                int Arity,
                                                unsigned int type_index,
                                                std::string type_name,
                                                bool per_frame) const {
          int pfi= per_frame?1:0;
          bool found=true;
          if (cache_.size() <= kc) {
            found=false;
          } else if (cache_[kc][pfi].size() <= type_index) {
            found=false;
          } else if (cache_[kc][pfi].is_null(type_index)) {
            found=false;
          }
          if (!found) {
            std::string nm= get_key_list_data_set_name(kcname,
                                                       Arity,
                                                       type_name,
                                                       per_frame);
            cache_.resize(std::max(cache_.size(),
                                   static_cast<size_t>(kc+1)));
            cache_[kc][pfi].resize(std::max(cache_[kc][pfi].size(),
                                            static_cast<size_t>(type_index+1)),
                                   NULL);
            cache_[kc][pfi].replace(type_index, new DS());
            cache_[kc][pfi][type_index].set(file, nm);
          }
          return cache_[kc][pfi][type_index];
        }
      };
      mutable Ints max_cache_;
      mutable set<std::string> known_data_sets_;
      boost::array<KeyNameDataSetCache,4> key_name_data_sets_;
      RMF_FOREACH_TYPE(RMF_HDF5_SHARED_DATA_TYPE);

      template <class TypeTraits>
        HDF5DataSetCacheD<StringTraits, 1>&
        get_key_list_data_set(unsigned int kc,
                              int arity,
                              bool per_frame) const {
        return key_name_data_sets_[arity-1].get(file_, kc,
                                                get_category_name(kc),
                                                arity,
                                                TypeTraits::get_index(),
                                                TypeTraits::get_name(),
                                                per_frame);
      }

      template <class TypeTraits>
        HDF5DataSetCacheD<TypeTraits, 2>&
        get_data_data_set(unsigned int kc, int arity) const {
        return get_data_set_i(TypeTraits(), kc, arity);
      }
      template <class TypeTraits>
        HDF5DataSetCacheD<TypeTraits, 3>&
        get_per_frame_data_data_set(unsigned int kc, int arity) const {
        return get_per_frame_data_set_i(TypeTraits(), kc, arity);
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

      template <class TypeTraits>
        typename TypeTraits::Types get_all_values_impl(unsigned int node,
                                                      Key<TypeTraits> k)
        const {
        RMF_BEGIN_FILE
          RMF_USAGE_CHECK(k.get_is_per_frame(),
                      "Using get_all_values on a key that is not per_frame.");
        unsigned int kc=k.get_category().get_index();
        int vi=get_index_from_cache<1>(node, kc);
        if (IndexTraits::get_is_null_value(vi)) {
          RMF_BEGIN_OPERATION
          int index= get_index(1, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[1-1].get_size();
          RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            return typename TypeTraits::Types();
          } else {
            vi=node_data_[1-1].get_value(HDF5DataSetIndexD<2>(node, index));
          }
          if (IndexTraits::get_is_null_value(vi)) {
            return typename TypeTraits::Types();
          } else {
            add_index_to_cache<1>(node, kc, vi);
          }
          RMF_END_OPERATION("getting value index");
        }
        {
          RMF_BEGIN_OPERATION
            HDF5DataSetCacheD<TypeTraits, 3> &ds
            = get_per_frame_data_data_set<TypeTraits>(kc,
                                                      1);
          HDF5DataSetIndexD<3> sz= ds.get_size();
          if (static_cast<hsize_t>(vi) >= sz[0]
              || static_cast<hsize_t>(k.get_index()) >= sz[1]) {
            return typename TypeTraits::Types();
          } else {
            return ds.get_row(HDF5DataSetIndexD<2>(vi, k.get_index()));
          }
          RMF_END_OPERATION("fetching data from per frame data set");
        }
        RMF_END_FILE(get_file_name());
      }

      template <class TypeTraits>
        typename TypeTraits::Type get_value_impl(unsigned int node,
                                                 Key<TypeTraits> k,
                                                 unsigned int frame) const {
        RMF_BEGIN_FILE
        unsigned int kc=k.get_category().get_index();
        bool per_frame= k.get_is_per_frame();
        int vi=get_index_from_cache<1>(node, kc);
        if (IndexTraits::get_is_null_value(vi)) {
          RMF_BEGIN_OPERATION
          int index= get_index(1, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[1-1].get_size();
          // deal with nodes added for sets
          if (static_cast<unsigned int>(nsz[0]) <= node) {
            return TypeTraits::get_null_value();
          }
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            return TypeTraits::get_null_value();
          } else {
            vi=node_data_[1-1].get_value(HDF5DataSetIndexD<2>(node, index));
          }
          if (IndexTraits::get_is_null_value(vi)) {
            return TypeTraits::get_null_value();
          } else {
            add_index_to_cache<1>(node, kc, vi);
          }
          RMF_END_OPERATION("getting value index");
        }
        {
          if (per_frame) {
            RMF_BEGIN_OPERATION
            HDF5DataSetCacheD<TypeTraits, 3> &ds
              = get_per_frame_data_data_set<TypeTraits>(kc,
                                                        1);
            HDF5DataSetIndexD<3> sz= ds.get_size();
            if (static_cast<hsize_t>(vi) >= sz[0]
                || static_cast<hsize_t>(k.get_index()) >= sz[1]
                || (frame >= static_cast<unsigned int>(sz[2]))) {
              return TypeTraits::get_null_value();
            } else {
              return ds.get_value(HDF5DataSetIndexD<3>(vi, k.get_index(),
                                                       frame));
            }
            RMF_END_OPERATION("fetching data from per frame data set")
          } else {
            RMF_BEGIN_OPERATION
            HDF5DataSetCacheD<TypeTraits,2> &ds
              = get_data_data_set<TypeTraits>(kc,
                                              1);
            HDF5DataSetIndexD<2> sz= ds.get_size();
            if (static_cast<hsize_t>(vi) >= sz[0]
                || static_cast<hsize_t>(k.get_index()) >= sz[1]) {
              return TypeTraits::get_null_value();
            } else {
              return ds.get_value(HDF5DataSetIndexD<2>(vi, k.get_index()));
            }
            RMF_END_OPERATION("fetching data from data set");
          }
        }
        RMF_END_FILE(get_file_name());
      }
      template <class TypeTraits>
        unsigned int get_number_of_frames(Key<TypeTraits> k) const {
        unsigned int kc= k.get_category().get_index();
        if (!get_is_per_frame(k)) {
          RMF_THROW(get_error_message("Attribue ", get_name(k),
                                          " does not have frames."),
                        UsageException);
          RMF_NO_RETURN(int);
        } else {
          HDF5DataSetCacheD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     1);
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
          RMF_BEGIN_OPERATION;
          unsigned int index= get_index(Arity, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[Arity-1].get_size();
          RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
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
          RMF_END_OPERATION("figuring out where to store value");
        }
        return vi;
      }

      template <class TypeTraits>
        void make_fit( HDF5DataSetCacheD<TypeTraits, 3> &ds,
                       int vi,
                       Key<TypeTraits> k, unsigned int frame) {
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
      template <class TypeTraits>
        void make_fit( HDF5DataSetCacheD<TypeTraits, 2> &ds,
                       int vi,
                       Key<TypeTraits> k) {
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
      template <class TypeTraits>
        void set_value_impl(unsigned int node, Key<TypeTraits> k,
                            typename TypeTraits::Type v, unsigned int frame) {
        RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(v),
                        "Cannot write sentry value to an RMF file.");
        unsigned int kc= k.get_category().get_index();
        bool per_frame= get_is_per_frame(k);
        int vi=get_index_set<1>(node, kc);
        if (per_frame) {
          RMF_BEGIN_OPERATION
            HDF5DataSetCacheD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     1);
          make_fit(ds, vi, k, frame);
          ds.set_value(HDF5DataSetIndexD<3>(vi, k.get_index(), frame), v);
          RMF_END_OPERATION("storing per frame value");
        } else {
          RMF_BEGIN_OPERATION
          HDF5DataSetCacheD<TypeTraits, 2> &ds
            =get_data_data_set<TypeTraits>(kc, 1);
          make_fit(ds, vi, k);
          ds.set_value(HDF5DataSetIndexD<2>(vi, k.get_index()), v);
          RMF_END_OPERATION("storing single value")
        }
        /*RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                               "Stored " << v << " but got "
                               << get_value(node, k, frame));*/
      }

      template <class TypeTraits>
        Key<TypeTraits> add_key_impl(int category_id,
                                       std::string name, bool per_frame) {
        audit_key_name(name);
        // check that it is unique
        RMF_BEGIN_OPERATION;
        {
          HDF5DataSetCacheD<StringTraits, 1> &nameds
            = get_key_list_data_set<TypeTraits>(category_id, 1,
                                                per_frame);
          unsigned int sz= nameds.get_size()[0];
          HDF5DataSetIndexD<1> index;
          for (unsigned int i=0; i< sz; ++i) {
            index[0]=i;
            RMF_USAGE_CHECK(nameds.get_value(index) != name,
                                get_error_message("Attribute name ", name,
                                             " already taken for that type."));
          }
        }
        RMF_END_OPERATION("checking that key is unique");
        RMF_BEGIN_OPERATION;
        HDF5DataSetCacheD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, 1,
                                              per_frame);
        HDF5DataSetIndexD<1> sz= nameds.get_size();
        int ret_index= sz[0];
        ++sz[0];
        nameds.set_size(sz);
        --sz[0];
        nameds.set_value(sz, name);
        return Key<TypeTraits>(category_id, ret_index, per_frame);
        RMF_END_OPERATION("appending key to list")
      }

      // create the data sets and add rows to the table
      template <class TypeTraits>
        unsigned int
        get_number_of_keys_impl(int category_id, bool per_frame) const {
        HDF5DataSetCacheD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, 1,
                                              per_frame);
        HDF5DataSetIndexD<1> sz= nameds.get_size();
        return sz[0];
      }

      template <class TypeTraits>
        std::string get_name_impl(Key<TypeTraits> k) const {
        unsigned int kc=k.get_category().get_index();
        HDF5DataSetCacheD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(kc, 1,
                                              k.get_is_per_frame());
        HDF5DataSetIndexD<1> index(k.get_index());
        return nameds.get_value(index);
      }
      template <class TypeTraits>
        Key<TypeTraits> get_key_impl(int category_id,
                                     std::string name,
                                     bool per_frame) const {
        HDF5DataSetCacheD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id, 1,
                                              per_frame);
        HDF5DataSetIndexD<1> size= nameds.get_size();
        for (unsigned int j=0; j< size[0]; ++j) {
          HDF5DataSetIndexD<1> index(j);
          std::string cur=nameds.get_value(index);
          if (cur== name) {
            return Key<TypeTraits>(category_id, j, per_frame);
          }
        }
        return Key<TypeTraits>();
      }

      void initialize_categories(int i, bool create);
      void initialize_keys(int i);
      void initialize_free_nodes();

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
      void init_link();
    public:
      RMF_FOREACH_TYPE(RMF_HDF5_SHARED_TYPE);

      HDF5Group get_group() const {
        return file_;
      }
      void flush() ;

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
        frames_hint_=i;
      }
      unsigned int get_number_of_frames() const;
      //
      int add_category(std::string name);
      unsigned int get_number_of_categories() const;
      std::string get_category_name(unsigned int kc) const  {
        RMF_USAGE_CHECK(category_names_[0].get_size()[0] > kc,
                        "No such category.");
        return category_names_[0].get_value(kc);
      }

      std::string get_description() const;
      void set_description(std::string str);

      void set_frame_name(std::string str);
      std::string get_frame_name() const;

       bool get_supports_locking() const {return false;}
       void reload();
       void set_current_frame(int frame);
    };

  } // namespace internal
} /* namespace RMF */


#endif /* RMF__INTERNAL_HDF_5SHARED_DATA_H */
