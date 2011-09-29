/**
 *  \file RMF/internal/shared.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_SHARED_H
#define IMPLIBRMF_INTERNAL_SHARED_H

#include "../RMF_config.h"
#include "../Key.h"
#include "../hdf5_types.h"
#include "../names.h"
#include "../HDF5Group.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/tuple/tuple.hpp>
#include <hdf5.h>
#include <algorithm>


namespace RMF {
  namespace internal {

    template <class T>
    inline std::ostream &operator<<(std::ostream &out,
                                    const std::vector<T> &o) {
      out << "[";
      for (unsigned int i=0; i< o.size(); ++i) {
        if (i != 0) out << ", ";
        out << o[i];
      }
      out << "]";
      return out;
    }


#define IMP_RMF_SHARED_DATA_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues)              \
    DataDataSetCache<Ucname##Traits, 2> lcname##_data_sets_;            \
    DataDataSetCache<Ucname##Traits, 3> per_frame_##lcname##_data_sets_; \
    HDF5DataSetD<Ucname##Traits, 2>&                                     \
    get_data_set_i(Ucname##Traits,                                      \
                   Category kc,                                         \
                   bool create_if_needed) const {                       \
      return lcname##_data_sets_.get(file_, kc, create_if_needed);      \
    }                                                                   \
    HDF5DataSetD<Ucname##Traits, 3>&                                     \
    get_per_frame_data_set_i(Ucname##Traits,                            \
                             Category kc,                               \
                             bool create_if_needed) const {             \
      return per_frame_##lcname##_data_sets_.get(file_, kc,             \
                                                 create_if_needed);     \
    }






    class RMFEXPORT SharedData: public boost::intrusive_ptr_object {
      // indexed first by per frame, then by
      // TypeInfo::get_index() then by ID
      // then by key.get_index()
      mutable HDF5Group file_;
      HDF5DataSetD<StringTraits, 1> names_;
      HDF5DataSetD<IndexTraits, 2> node_data_;
      HDF5DataSetD<IndexTraits, 2> bond_data_;
      Ints free_ids_;
      Ints free_bonds_;
      std::vector<void*> association_;
      map<void*, int> back_association_;
      unsigned int frames_hint_;

      // caches
      template <class TypeTraits, unsigned int D>
        class DataDataSetCache {
        typedef HDF5DataSetD<TypeTraits, D> DS;
        mutable typename std::vector<DS > cache_;
        mutable HDF5DataSetD<TypeTraits, D> null_;
      public:
        HDF5DataSetD<TypeTraits,D>& get(HDF5Group file,
                                        Category kc,
                                       bool create_if_needed) const {
          bool found=true;
          if (cache_.size() <= kc.get_index()) {
            found=false;
          } else if (cache_[kc.get_index()] ==null_) {
            found=false;
          }
          if (!found) {
            std::string nm=get_data_data_set_name(kc,
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
                                   static_cast<size_t>(kc.get_index()+1)));
            cache_[kc.get_index()]=ds;
          }
          return cache_[kc.get_index()];
        }
      };
      class KeyNameDataSetCache {
        // category, type, per_frame
        typedef HDF5DataSetD<StringTraits, 1> DS;
        typedef std::pair<DS, DS> Pair;
        mutable std::vector<std::vector< Pair > > cache_;
        mutable HDF5DataSetD<StringTraits, 1> null_;
      public:
        HDF5DataSetD<StringTraits, 1>& get(HDF5Group file,
                                       Category kc,
                                       unsigned int type_index,
                                       std::string type_name,
                                       bool per_frame,
                                       bool create_if_needed) const {
          bool found=true;
          if (cache_.size() <= kc.get_index()) {
            found=false;
          } else if (cache_[kc.get_index()].size() <= type_index) {
            found=false;
          } else if ((per_frame
                      && cache_[kc.get_index()][type_index].first
                      ==null_)
                     || (!per_frame
                         && cache_[kc.get_index()][type_index].second
                         ==null_)) {
            found=false;
          }
          if (!found) {
            std::string nm= get_key_list_data_set_name(kc,
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
                                   static_cast<size_t>(kc.get_index()+1)));
            cache_[kc.get_index()]
              .resize(std::max(cache_[kc.get_index()].size(),
                               static_cast<size_t>(type_index+1)));
            if (per_frame) {
              cache_[kc.get_index()][type_index].first=ds;
            } else {
              cache_[kc.get_index()][type_index].second=ds;
            }
          }
          if (per_frame) {
            return cache_[kc.get_index()][type_index].first;
          } else {
            return cache_[kc.get_index()][type_index].second;
          }
        }
      };
      mutable Ints max_cache_;
      mutable set<std::string> known_data_sets_;
      mutable int last_node_;
      mutable Category last_category_;
      mutable int last_vi_;
      KeyNameDataSetCache key_name_data_sets_;
      IMP_RMF_FOREACH_TYPE(IMP_RMF_SHARED_DATA_TYPE);

      template <class TypeTraits>
        HDF5DataSetD<StringTraits, 1>& get_key_list_data_set(Category kc,
                                                             bool per_frame,
                                                     bool create_if_needed)
        const {
        return key_name_data_sets_.get(file_, kc, TypeTraits::get_index(),
                                       TypeTraits::get_name(),
                                       per_frame, create_if_needed);
      }

      template <class TypeTraits>
        HDF5DataSetD<TypeTraits, 2>&
        get_data_data_set(Category kc,
                          bool create_if_needed) const {
        return get_data_set_i(TypeTraits(), kc, create_if_needed);
      }
      template <class TypeTraits>
        HDF5DataSetD<TypeTraits, 3>&
        get_per_frame_data_data_set(Category kc,
                          bool create_if_needed) const {
        return get_per_frame_data_set_i(TypeTraits(), kc, create_if_needed);
      }

      enum Indexes {TYPE=0, CHILD=1, SIBLING=2, FIRST_KEY=3};

      unsigned int get_index(Category k) const {
        return k.get_index()+FIRST_KEY;
      }
      void check_node(unsigned int node) const;
      void audit_key_name(std::string name) const;
      void audit_node_name(std::string name) const;
      unsigned int get_column_maximum(Category cat) const {
        if (max_cache_.size() > cat.get_index()
            && max_cache_[cat.get_index()]>-2) {
          return max_cache_[cat.get_index()];
        }
        HDF5DataSetIndexD<2> sz= node_data_.get_size();
        int mx=-1;
        int index= get_index(cat);
        for (unsigned int i=0; i< sz[0]; ++i) {
          mx= std::max(mx, node_data_.get_value(HDF5DataSetIndexD<2>(i,
                                                                     index)));
        }
        max_cache_.resize(std::max(max_cache_.size(),
                                   static_cast<size_t>(cat.get_index()+1)), -2);
        max_cache_[cat.get_index()]=mx;
        return mx;
      }
    public:
      template <class TypeTraits>
        typename TypeTraits::Type get_value_always(unsigned int node,
                                                   Key<TypeTraits> k,
                                                   unsigned int frame) const {
        int vi=-1;
        bool per_frame= k.get_is_per_frame();
        if (static_cast<int>(node)== last_node_
            && k.get_category()== last_category_) {
          vi= last_vi_;
          if (IndexTraits::get_is_null_value(vi)) {
            return TypeTraits::get_null_value();
          }
        } else {
          int index= get_index(k.get_category());
          HDF5DataSetIndexD<2> nsz= node_data_.get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          last_node_=node;
          last_category_=k.get_category();
          if (nsz[1] <= static_cast<hsize_t>(index)) {
            last_vi_= IndexTraits::get_null_value();
            return TypeTraits::get_null_value();
          } else {
            vi=node_data_.get_value(HDF5DataSetIndexD<2>(node, index));
          }
          last_vi_= vi;
        }
        if (IndexTraits::get_is_null_value(vi)) {
          return TypeTraits::get_null_value();
        } else {
          if (per_frame) {
            HDF5DataSetD<TypeTraits, 3> &ds
              = get_per_frame_data_data_set<TypeTraits>(k.get_category(),
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
          } else {
            HDF5DataSetD<TypeTraits,2> &ds
              = get_data_data_set<TypeTraits>(k.get_category(),
                                              false);
            if (!ds) return TypeTraits::get_null_value();
            HDF5DataSetIndexD<2> sz= ds.get_size();
            if (static_cast<hsize_t>(vi) >= sz[0]
                || static_cast<hsize_t>(k.get_index()) >= sz[1]) {
              return TypeTraits::get_null_value();
            } else {
              return ds.get_value(HDF5DataSetIndexD<2>(vi, k.get_index()));
            }
          }
        }
      }
      HDF5Group get_group() const {
        return file_;
      }
      void flush() const {
        IMP_HDF5_CALL(H5Fflush(file_.get_handle(), H5F_SCOPE_GLOBAL));
      }

      Categories get_categories() const;
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
      template <class TypeTraits>
        unsigned int get_number_of_frames(Key<TypeTraits> k) const {
        if (!get_is_per_frame(k)) return 0;
        else {
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(k.get_category(),
                                                     false);
          if (!ds) return 0;
          HDF5DataSetIndexD<3> sz= ds.get_size();
          return sz[2];
        }
      }
      void add_bond( int ida,  int idb,  int type);
      unsigned int get_number_of_bonds() const;
      boost::tuple<int,int,int> get_bond(unsigned int i) const;

      template <class TypeTraits>
        bool get_is_per_frame(Key<TypeTraits> k) const {
        return k.get_is_per_frame();
      }

      template <class TypeTraits>
        typename TypeTraits::Type get_value(unsigned int node,
                                            Key<TypeTraits> k,
                                            unsigned int frame) const {
        typename TypeTraits::Type ret= get_value_always(node, k, frame);
        IMP_RMF_USAGE_CHECK(!TypeTraits::get_is_null_value(ret),
                            "Invalid value requested");
        return ret;
      }

      template <class TypeTraits>
        bool get_has_value(unsigned int node, Key<TypeTraits> k,
                           unsigned int frame) const {
        return !TypeTraits::get_is_null_value(get_value_always(node, k, frame));
      }

      template <class TypeTraits>
        void set_value(unsigned int node, Key<TypeTraits> k,
                       typename TypeTraits::Type v, unsigned int frame) {
        bool per_frame= get_is_per_frame(k);
        int vi=IndexTraits::get_null_value();
        if (static_cast<int>(node)== last_node_ && k.get_category()
            == last_category_) {
          vi= last_vi_;
        }
        if (IndexTraits::get_is_null_value(vi)) {
          unsigned int index= get_index(k.get_category());
          HDF5DataSetIndexD<2> nsz= node_data_.get_size();
          IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                              "Invalid node used");
          if (nsz[1] <=index) {
            HDF5DataSetIndexD<2> newsz= nsz;
            newsz[1]= index+1;
            node_data_.set_size(newsz);
          }
          // now it is big enough
          // make sure the target table is there
          /*if (!get_has_data_set(nm)) {
            file_.add_data_set<TypeTraits>(nm, (per_frame?3:2));
            }*/
          // now we have the index and the data set is there
          vi= node_data_.get_value(HDF5DataSetIndexD<2>(node, index));
          if (IndexTraits::get_is_null_value(vi)) {
            vi= get_column_maximum(k.get_category())+1;
            node_data_.set_value(HDF5DataSetIndexD<2>(node, index), vi);
            max_cache_[k.get_category().get_index()]=vi;
          }
          last_vi_=vi;
        }
        if (per_frame) {
          HDF5DataSetD<TypeTraits, 3> &ds
            =get_per_frame_data_data_set<TypeTraits>(k.get_category(), true);
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
        } else {
          HDF5DataSetD<TypeTraits, 2> &ds
            =get_data_data_set<TypeTraits>(k.get_category(), true);
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
        }
        IMP_RMF_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                               "Stored " << v << " but got "
                               << get_value(node, k, frame));
      }

      template <class TypeTraits>
        Key<TypeTraits> add_key(Category category_id,
                                std::string name, bool per_frame) {
        audit_key_name(name);
        // check that it is unique
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
          HDF5DataSetD<StringTraits, 1> &nameds
            = get_key_list_data_set<TypeTraits>(category_id,
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
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(category_id,
                                              per_frame,
                                              true);
        HDF5DataSetIndexD<1> sz= nameds.get_size();
        int ret_index= sz[0];
        ++sz[0];
        nameds.set_size(sz);
        --sz[0];
        nameds.set_value(sz, name);
        return Key<TypeTraits>(category_id, ret_index, per_frame);
      }

      // create the data sets and add rows to the table
      template <class TypeTraits>
        std::vector<Key<TypeTraits> > get_keys(Category category_id) const {
        std::vector<Key<TypeTraits> > ret;
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
          HDF5DataSetD<StringTraits, 1>& nameds
            = get_key_list_data_set<TypeTraits>(category_id,
                                                per_frame, false);
          if (!nameds) continue;
          HDF5DataSetIndexD<1> sz= nameds.get_size();
          for (unsigned int j=0; j< sz[0]; ++j) {
            ret.push_back(Key<TypeTraits>(category_id, j, per_frame));
          }
        }
        return ret;
      }

      template <class TypeTraits>
        std::string get_name(Key<TypeTraits> k) {
        HDF5DataSetD<StringTraits, 1>& nameds
          = get_key_list_data_set<TypeTraits>(k.get_category(),
                                              k.get_is_per_frame(),
                                              false);
        IMP_RMF_USAGE_CHECK(nameds, "No keys of the desired category found");
        HDF5DataSetIndexD<1> index(k.get_index());
        return nameds.get_value(index);
      }

      template <class TypeTraits>
        Key<TypeTraits> get_key(Category category_id, std::string name) {
        for (unsigned int i=0; i< 2; ++i) {
          bool per_frame=(i==0);
          HDF5DataSetD<StringTraits, 1>& nameds
            = get_key_list_data_set<TypeTraits>(category_id, per_frame,
                                                false);
          if (!nameds) {
            return Key<TypeTraits>();
          }
          HDF5DataSetIndexD<1> size= nameds.get_size();
          for (unsigned int j=0; j< size[0]; ++j) {
            HDF5DataSetIndexD<1> index(j);
            std::string cur=nameds.get_value(index);
            if (cur== name) {
              return Key<TypeTraits>(category_id, j, per_frame);
            }
          }
        }
        return Key<TypeTraits>();
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
    };




  } // namespace internal
} /* namespace RMF */


#endif /* IMPLIBRMF_INTERNAL_SHARED_H */
