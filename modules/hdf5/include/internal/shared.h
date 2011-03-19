/**
 *  \file IMP/utility.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_INTERNAL_SHARED_H
#define IMPHDF5_INTERNAL_SHARED_H

#include "../hdf5_config.h"
#include "../Key.h"
#include "../hdf5_types.h"
#include "../names.h"
#include "../hdf5_wrapper.h"
#include <boost/tuple/tuple.hpp>
#include <hdf5.h>
#include <algorithm>


IMPHDF5_BEGIN_INTERNAL_NAMESPACE

class IMPHDF5EXPORT SharedData: public RefCounted {
  // indexed first by per frame, then by
  // TypeInfo::get_index() then by ID
  // then by key.get_index()
  mutable HDF5Group file_;
  HDF5DataSet<StringTraits> names_;
  HDF5DataSet<IndexTraits> node_data_;
  HDF5DataSet<IndexTraits> bond_data_;
  std::vector<int> free_ids_;
  std::vector<int> free_bonds_;
  std::vector<void*> association_;
  IMP::internal::Map<void*, int> back_association_;

  // caches
  mutable std::vector<int> max_cache_;
  mutable IMP::internal::Set<std::string> known_data_sets_;
  mutable int last_node_;
  mutable KeyCategory last_category_;
  mutable int last_vi_;
  mutable IMP::internal::Map<std::string, HDF5DataSet<IntTraits> >
    int_data_sets_;
  mutable IMP::internal::Map<std::string, HDF5DataSet<StringTraits> >
    string_data_sets_;
  mutable IMP::internal::Map<std::string, HDF5DataSet<IndexTraits> >
    index_data_sets_;
  mutable IMP::internal::Map<std::string, HDF5DataSet<FloatTraits> >
    float_data_sets_;
  mutable IMP::internal::Map<std::string, HDF5DataSet<NodeIDTraits> >
    node_id_data_sets_;

  template <class Traits>
    HDF5DataSet<Traits>
    generic_get_data_set(IMP::internal::Map<std::string,
                                            HDF5DataSet<Traits> > &cache,
                         std::string name, int dims) const {
    typename IMP::internal::Map<std::string,
                                HDF5DataSet<Traits> >::iterator it
      = cache.find(name);
    if (it == cache.end()) {
      HDF5DataSet<Traits> ds;
      if (file_.get_has_child(name)) {
        ds= file_.get_child_data_set<Traits>(name);
      } else {
        ds= file_.add_child_data_set<Traits>(name, dims);
      }
      cache[name]=ds;
      return ds;
    } else {
      return it->second;
    }
  }

  HDF5DataSet<IntTraits> get_data_set(IntTraits,
                                      std::string name, int dims) const {
    return generic_get_data_set(int_data_sets_, name, dims);
  }
  HDF5DataSet<FloatTraits> get_data_set(FloatTraits,
                                        std::string name, int dims) const {
    return generic_get_data_set(float_data_sets_, name, dims);
  }
  HDF5DataSet<StringTraits> get_data_set(StringTraits,
                                         std::string name, int dims) const {
    return generic_get_data_set(string_data_sets_, name, dims);
  }
  HDF5DataSet<IndexTraits> get_data_set(IndexTraits,
                                        std::string name, int dims) const {
    return generic_get_data_set(index_data_sets_, name, dims);
  }
  HDF5DataSet<NodeIDTraits> get_data_set(NodeIDTraits,
                                        std::string name, int dims) const {
    return generic_get_data_set(node_id_data_sets_, name, dims);
  }

  enum Indexes {TYPE=0, CHILD=1, SIBLING=2, FIRST_KEY=3};
  typedef std::vector<int> Ind;
  static Ind make_index(int i) {
    return Ind(1, i);
  }
  static Ind make_index(int i, int j) {
    Ind ret(2);
    ret[0]=i;
    ret[1]=j;
    return ret;
  }
  static Ind make_index(int i, int j, int k) {
    Ind ret(3);
    ret[0]=i;
    ret[1]=j;
    ret[2]=k;
    return ret;
  }
  unsigned int get_index(KeyCategory k) const {
    return k.get_index()+FIRST_KEY;
  }
  void check_node(unsigned int node) const;


  bool get_has_data_set(std::string name) const {
    if (known_data_sets_.find(name) != known_data_sets_.end()) {
      return true;
    } else {
      bool ret= file_.get_has_child(name);
      if (ret) {
        known_data_sets_.insert(name);
      }
      return ret;
    }
  }

  unsigned int get_column_maximum(KeyCategory cat) const {
    if (max_cache_.size() > cat.get_index()
        && max_cache_[cat.get_index()]>-2) {
      return max_cache_[cat.get_index()];
    }
    Ints sz= node_data_.get_size();
    int mx=-1;
    int index= get_index(cat);
    for (int i=0; i< sz[0]; ++i) {
      mx= std::max(mx, node_data_.get_value(make_index(i, index)));
    }
    max_cache_.resize(std::max(max_cache_.size(),
                               static_cast<size_t>(cat.get_index()+1)), -2);
    max_cache_[cat.get_index()]=mx;
    return mx;
  }

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
      Ints nsz= node_data_.get_size();
      IMP_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                      "Invalid node used");
      last_node_=node;
      last_category_=k.get_category();
      if (nsz[1] <=static_cast<int>(index)) {
        last_vi_= IndexTraits::get_null_value();
        return TypeTraits::get_null_value();
      } else {
        vi=node_data_.get_value(make_index(node, index));
      }
      last_vi_= vi;
    }
    if (IndexTraits::get_is_null_value(vi)) {
      return TypeTraits::get_null_value();
    } else {
      std::string nm= get_data_data_set_name<TypeTraits>(k.get_category(),
                                                         per_frame);
      if (!get_has_data_set(nm)) {
        return TypeTraits::get_null_value();
      } else {
        HDF5DataSet<TypeTraits> ds= get_data_set(TypeTraits(),
                                                 nm, (per_frame?3:2));
        Ints sz= ds.get_size();
        if (vi >= sz[0] || k.get_index() >= sz[1]
            || (per_frame && frame >= static_cast<unsigned int>(sz[2]))) {
          return TypeTraits::get_null_value();
        } else if (per_frame){
          return ds.get_value(make_index(vi, k.get_index(), frame));
        } else {
          return ds.get_value(make_index(vi, k.get_index()));
        }
      }
    }
  }
 public:
  void set_association(int id, void *d) {
    if (association_.size() <= static_cast<unsigned int>(id)) {
      association_.resize(id+1, NULL);
    }
    association_[id]=d;
    back_association_[d]=id;
  }
  void* get_association(int id) const {
    IMP_USAGE_CHECK(static_cast<unsigned int>(id) < association_.size(),
                    "Unassociated id");
    return association_[id];
  }
  int get_association(void* d) const {
    IMP_USAGE_CHECK(back_association_.find(d) != back_association_.end(),
                    "Unassociated id");
    return back_association_.find(d)->second;
  }
  template <class TypeTraits>
  unsigned int get_number_of_frames(Key<TypeTraits> k) const {
    if (!get_is_per_frame(k)) return 0;
    else {
      std::string nm=get_data_data_set_name<TypeTraits>(k.get_category(), true);
      HDF5DataSet<TypeTraits> ds=get_data_set(TypeTraits(), nm, 3);
      Ints sz= ds.get_size();
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
    typename TypeTraits::Type get_value(unsigned int node, Key<TypeTraits> k,
                                        unsigned int frame) const {
    typename TypeTraits::Type ret= get_value_always(node, k, frame);
    IMP_USAGE_CHECK(!TypeTraits::get_is_null_value(ret),
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
    std::string nm=get_data_data_set_name<TypeTraits>(k.get_category(),
                                                      per_frame);
    if (static_cast<int>(node)== last_node_ && k.get_category()
        == last_category_) {
      vi= last_vi_;
    }
    if (IndexTraits::get_is_null_value(vi)) {
      int index= get_index(k.get_category());
      Ints nsz= node_data_.get_size();
      IMP_USAGE_CHECK(static_cast<unsigned int>(nsz[0]) > node,
                      "Invalid node used");
      if (nsz[1] <=static_cast<int>(index)) {
        Ints newsz= nsz;
        newsz[1]= index+1;
        node_data_.set_size(newsz);
      }
      // now it is big enough
      // make sure the target table is there
      /*if (!get_has_data_set(nm)) {
        file_.add_data_set<TypeTraits>(nm, (per_frame?3:2));
        }*/
      // now we have the index and the data set is there
      vi= node_data_.get_value(make_index(node, index));
      if (IndexTraits::get_is_null_value(vi)) {
        vi= get_column_maximum(k.get_category())+1;
        node_data_.set_value(make_index(node, index), vi);
        max_cache_[k.get_category().get_index()]=vi;
      }
      last_vi_=vi;
    }
    HDF5DataSet<TypeTraits> ds=get_data_set(TypeTraits(), nm, (per_frame?3:2));
    Ints sz= ds.get_size();
    bool delta=false;
    if (sz[0] <= static_cast<int>(vi)) {
      sz[0] = vi+1;
      delta=true;
    }
    if (sz[1] <= k.get_index()) {
      sz[1]= k.get_index()+1;
      delta=true;
    }
    if (per_frame && static_cast<unsigned int>(sz[2]) <= frame) {
      sz[2] =frame+1;
      delta=true;
    }
    if (delta) {
      ds.set_size(sz);
    }
    if (get_is_per_frame(k)) {
      ds.set_value(make_index(vi, k.get_index(), frame), v);
    } else {
      ds.set_value(make_index(vi, k.get_index()), v);
    }
    IMP_INTERNAL_CHECK(get_value(node, k, frame) ==v,
                       "Stored " << v << " but got "
                       << get_value(node, k, frame));
  }
  template <class TypeTraits>
    Key<TypeTraits> add_key(KeyCategory category_id,
                       std::string name, bool per_frame) {
    IMP_USAGE_CHECK(!name.empty(), "Key name cannot be empty");
    // check that it is unique
    for (unsigned int i=0; i< 2; ++i) {
      bool per_frame=(i==0);
      std::string nm= get_key_list_attribute_name<TypeTraits>(category_id,
                                                              per_frame);
      std::vector<std::string> names = file_.get_attribute<StringTraits>(nm);
      for (unsigned int i=0; i< names.size(); ++i) {
        IMP_USAGE_CHECK(names[i] != name, "Attribute name " << name
                        << " already taken for that type.");
      }
    }
    std::string nm= get_key_list_attribute_name<TypeTraits>(category_id,
                                                            per_frame);
    std::vector<std::string> names = file_.get_attribute<StringTraits>(nm);
    int ret_index= names.size();
    names.push_back(name);
    file_.set_attribute<StringTraits>(nm, names);
    return Key<TypeTraits>(category_id, ret_index, per_frame);
  }
      // create the data sets and add rows to the table
  template <class TypeTraits>
    std::vector<Key<TypeTraits> > get_keys(KeyCategory category_id) const {
    std::vector<Key<TypeTraits> > ret;
    for (unsigned int i=0; i< 2; ++i) {
      bool per_frame=(i==0);
      std::string nm= get_key_list_attribute_name<TypeTraits>(category_id,
                                                              per_frame);
      std::vector<std::string> names = file_.get_attribute<StringTraits>(nm);
      for (unsigned int i=0; i< names.size(); ++i) {
        ret.push_back(Key<TypeTraits>(category_id, i, per_frame));
      }
    }
    return ret;
  }
  template <class TypeTraits>
  std::string get_name(Key<TypeTraits> k) {
    std::string attr_nm
      =get_key_list_attribute_name<TypeTraits>(k.get_category(),
                                               k.get_is_per_frame());
    return file_.get_attribute<StringTraits>(attr_nm)[k.get_index()];
  }

  template <class TypeTraits>
    Key<TypeTraits> get_key(KeyCategory category_id, std::string name) {
    for (unsigned int i=0; i< 2; ++i) {
      bool per_frame=(i==0);
      std::string nm= get_key_list_attribute_name<TypeTraits>(category_id,
                                                              per_frame);
      std::vector<std::string> names = file_.get_attribute<StringTraits>(nm);
      for (unsigned int i=0; i< names.size(); ++i) {
        if (names[i] == name) {
          return Key<TypeTraits>(category_id, i, per_frame);
        }
      }
    }
    return Key<TypeTraits>();
  }

  SharedData(std::string name, bool clear);
  int add_node(std::string name, unsigned int type);
  int get_first_child(unsigned int node) const;
  int get_sibling(unsigned int node) const;
  void set_first_child(unsigned int node, int child);
  void set_sibling(unsigned int node, int sibling);
  std::string get_name(unsigned int node) const;
  unsigned int get_type(unsigned int node) const;


  int add_child(int node, std::string name, int t);
  Ints get_children(int node) const;
};




IMPHDF5_END_INTERNAL_NAMESPACE


#endif /* IMPHDF5_INTERNAL_SHARED_H */
