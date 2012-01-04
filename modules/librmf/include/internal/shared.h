/**
 *  \file RMF/internal/shared.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_SHARED_H
#define IMPLIBRMF_INTERNAL_SHARED_H

#include "../RMF_config.h"
#include "../Key.h"
#include "../hdf5_types.h"
#include "../names.h"
#include "../HDF5Group.h"
#include "../HDF5File.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <hdf5.h>
#include <algorithm>


namespace RMF {
  namespace internal {

#define IMP_RMF_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue, \
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






    class RMFEXPORT SharedData: public boost::intrusive_ptr_object {
      // indexed first by per frame, then by
      // TypeInfo::get_index() then by ID
      // then by key.get_index()
      mutable HDF5Group file_;
      HDF5DataSetD<StringTraits, 1> node_names_;
      boost::array<HDF5DataSetD<StringTraits, 1>, 4> category_names_;
      boost::array<Strings, 4> category_names_cache_;
      boost::array<HDF5DataSetD<IndexTraits, 2>, 4> node_data_;
      boost::array<Ints,4> free_ids_;
      vector<void*> association_;
      map<void*, int> back_association_;
      unsigned int frames_hint_;

      // caches
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
                file.add_child_data_set<TypeTraits, D>(nm);
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
                HDF5DataSetD<StringTraits, 1> ds
                  = file.add_child_data_set<StringTraits, 1>(nm);
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
      mutable boost::array<int,4> last_node_;
      mutable boost::array<int,4> last_category_;
      mutable boost::array<int,4> last_vi_;
      boost::array<KeyNameDataSetCache,4> key_name_data_sets_;
      IMP_RMF_FOREACH_TYPE(IMP_RMF_SHARED_DATA_TYPE);

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
      void audit_key_name(std::string name) const;
      void audit_node_name(std::string name) const;
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
    public:
      template <class TypeTraits, int Arity>
        typename TypeTraits::Type get_value_always(unsigned int node,
                                                   Key<TypeTraits,Arity> k,
                                                   unsigned int frame) const {
        IMP_RMF_BEGIN_FILE
        int vi=-1;
        unsigned int kc=k.get_category().get_index();
        bool per_frame= k.get_is_per_frame();
        if (static_cast<int>(node)== last_node_[Arity-1]
            && static_cast<int>(kc)== last_category_[Arity-1]) {
          vi= last_vi_[Arity-1];
          if (IndexTraits::get_is_null_value(vi)) {
            return TypeTraits::get_null_value();
          }
        } else {
          IMP_RMF_BEGIN_OPERATION
          int index= get_index(Arity, kc);
          HDF5DataSetIndexD<2> nsz= node_data_[Arity-1].get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          last_node_[Arity-1]=node;
          last_category_[Arity-1]=kc;
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            last_vi_[Arity-1]= IndexTraits::get_null_value();
            return TypeTraits::get_null_value();
          } else {
            vi=node_data_[Arity-1].get_value(HDF5DataSetIndexD<2>(node, index));
          }
          last_vi_[Arity-1]= vi;
          IMP_RMF_END_OPERATION("getting value index");
        }
        if (IndexTraits::get_is_null_value(vi)) {
          return TypeTraits::get_null_value();
        } else {
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
            IMP_RMF_END_OPERATION("fetching data from data set")
          }
        }
        IMP_RMF_END_FILE(get_file_name());
      }
      HDF5Group get_group() const {
        return file_;
      }
      void flush() const {
        IMP_HDF5_CALL(H5Fflush(file_.get_handle(), H5F_SCOPE_GLOBAL));
      }
      void set_association(int id, void *d, bool overwrite) {
        IMP_RMF_USAGE_CHECK(d, "NULL association");
        if (association_.size() <= static_cast<unsigned int>(id)) {
          association_.resize(id+1, NULL);
        }
        IMP_RMF_USAGE_CHECK(overwrite || !association_[id],
                            "Associations can only be set once");
        if (overwrite && association_[id]) {
          void *old=association_[id];
          if (back_association_[old]==id) {
            back_association_.erase(old);
          }
        }
        association_[id]=d;
        back_association_[d]=id;
      }
      bool get_has_association(void* d) const {
        return back_association_.find(d) != back_association_.end();
      }
      void* get_association(int id) const {
        IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(id) < association_.size(),
                            "Unassociated id");
        return association_[id];
      }
      std::string get_file_name() const {
        return file_.get_file().get_name();
      }
      int get_association(void* d) const {
        if (back_association_.find(d) == back_association_.end()) {
          IMP_RMF_IF_CHECK {
            for (unsigned int i=0; i< association_.size(); ++i) {
              IMP_RMF_INTERNAL_CHECK(association_[i] != d,
                                     "Association not in map, but found: "
                                     << d << " at " << i);
            }
          }
          IMP_RMF_USAGE_CHECK(back_association_.find(d)
                              != back_association_.end(),
                              "Unassociated id from "
                              << back_association_.size());
        }
        return back_association_.find(d)->second;
      }
      template <class TypeTraits, int Arity>
        unsigned int get_number_of_frames(Key<TypeTraits, Arity> k) const {
        unsigned int kc= k.get_category().get_index();
        if (!get_is_per_frame(k)) {
          IMP_RMF_THROW("Attribue " << get_name(k)
                        << " does not have frames.",
                        UsageException);
        } else {
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     k.get_arity(),
                                                     false);
          if (!ds) {
            IMP_RMF_THROW("Attribute " << get_name(k)
                          << " does not have any data.",
                          UsageException);
          }
          HDF5DataSetIndexD<3> sz= ds.get_size();
          return sz[2];
        }
      }
      void add_bond( int ida,  int idb,  int type);
      unsigned int get_number_of_bonds() const;
      boost::tuple<int,int,int> get_bond(unsigned int i) const;

      template <class TypeTraits, int Arity>
        bool get_is_per_frame(Key<TypeTraits, Arity> k) const {
        return k.get_is_per_frame();
      }

      template <class TypeTraits, int Arity>
        typename TypeTraits::Type get_value(unsigned int node,
                                            Key<TypeTraits, Arity> k,
                                            unsigned int frame) const {
        typename TypeTraits::Type ret= get_value_always(node, k, frame);
        IMP_RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(ret),
                            "Invalid value requested");
        return ret;
      }

      template <class TypeTraits, int Arity>
        bool get_has_value(unsigned int node, Key<TypeTraits, Arity> k,
                           unsigned int frame) const {
        return !TypeTraits::get_is_null_value(get_value_always(node, k, frame));
      }

      template <class TypeTraits, int Arity>
        void set_value(unsigned int node, Key<TypeTraits, Arity> k,
                       typename TypeTraits::Type v, unsigned int frame) {
        unsigned int kc= k.get_category().get_index();
        bool per_frame= get_is_per_frame(k);
        int vi=IndexTraits::get_null_value();
        if (static_cast<int>(node)== last_node_[Arity-1]
            && static_cast<int>(kc) == last_category_[Arity-1]) {
          vi= last_vi_[Arity-1];
        }
        if (IndexTraits::get_is_null_value(vi)) {
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
          last_vi_[Arity-1]=vi;
          IMP_RMF_END_OPERATION("figuring out where to store value");
        }
        if (per_frame) {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(kc,
                                                     k.get_arity(), true);
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
          ds.set_value(HDF5DataSetIndexD<3>(vi, k.get_index(), frame), v);
          IMP_RMF_END_OPERATION("storing per frame value");
        } else {
          IMP_RMF_BEGIN_OPERATION
          HDF5DataSetD<TypeTraits, 2> &ds
            =get_data_data_set<TypeTraits>(kc, k.get_arity(),
                                           true);
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
          ds.set_value(HDF5DataSetIndexD<2>(vi, k.get_index()), v);
          IMP_RMF_END_OPERATION("storing single value")
        }
        /*IMP_RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                               "Stored " << v << " but got "
                               << get_value(node, k, frame));*/
      }

      template <class TypeTraits, int Arity>
        Key<TypeTraits, Arity> add_key(int category_id,
                                       std::string name, bool per_frame) {
        audit_key_name(name);
        // check that it is unique
        IMP_RMF_BEGIN_OPERATION;
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
          HDF5DataSetD<StringTraits, 1> &nameds
            = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                                per_frame, true);
          unsigned int sz= nameds.get_size()[0];
          HDF5DataSetIndexD<1> index;
          for (unsigned int i=0; i< sz; ++i) {
            index[0]=i;
            IMP_RMF_USAGE_CHECK(nameds.get_value(index) != name,
                                "Attribute name " << name
                                << " already taken for that type.");
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
        vector<Key<TypeTraits, Arity> >
        get_keys(int category_id) const {
        vector<Key<TypeTraits, Arity> > ret;
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
          HDF5DataSetD<StringTraits, 1>& nameds
            = get_key_list_data_set<TypeTraits>(category_id, Arity,
                                                per_frame, false);
          if (!nameds) continue;
          HDF5DataSetIndexD<1> sz= nameds.get_size();
          for (unsigned int j=0; j< sz[0]; ++j) {
            ret.push_back(Key<TypeTraits, Arity>(category_id, j, per_frame));
          }
        }
        return ret;
      }

      template <class TypeTraits, int Arity>
        std::string get_name(Key<TypeTraits, Arity> k) const {
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
        Key<TypeTraits, Arity> get_key(int category_id,
                                       std::string name) const {
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
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
        }
        return Key<TypeTraits, Arity>();
      }


      SharedData(HDF5Group g, bool create);
      ~SharedData();
      int add_node(std::string name, unsigned int type);
      int get_first_child(unsigned int node) const;
      int get_sibling(unsigned int node) const;
      void set_first_child(unsigned int node, int child);
      void set_sibling(unsigned int node, int sibling);
      std::string get_name(unsigned int node) const;
      void set_name(unsigned int node, std::string name);
      unsigned int get_type(unsigned int node) const;


      int add_child(int node, std::string name, int t);
      Ints get_children(int node) const;
      void save_frames_hint(int i) {
        frames_hint_=i;
      }

      void check_set(int arity, unsigned int index) const;
      unsigned int get_number_of_sets(int arity) const;
      RMF::Indexes get_set_indexes(int Arity) const;
      unsigned int add_set( RMF::Indexes nis, int t);
      unsigned int get_set_member(int Arity, unsigned int index,
                                    int member_index) const;
      unsigned int get_set_type(int Arity, unsigned int index) const;

      int add_category(int Arity, std::string name);
      int get_category(int Arity, std::string name) const;
      Strings get_category_names(int Arity) const;
      Ints get_categories(int Arity) const;
      std::string get_category_name(int Arity, unsigned int kc) const  {
        IMP_RMF_USAGE_CHECK(category_names_cache_.size()
                            > static_cast<unsigned int>(Arity),
                            "No categories with arity " << Arity);
        IMP_RMF_USAGE_CHECK(category_names_cache_[Arity-1].size() > kc,
                            "No such category with arity " << Arity);
        return category_names_cache_[Arity-1][kc];
      }

      void initialize_categories(int i, bool create);
      void initialize_keys(int i);
      void initialize_free_nodes();
      void validate() const;
    };




  } // namespace internal
} /* namespace RMF */


#endif /* IMPLIBRMF_INTERNAL_SHARED_H */
