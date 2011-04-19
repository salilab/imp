/**
 *  \file IMP/utility.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_HDF5_WRAPPER_H
#define IMPHDF5_HDF5_WRAPPER_H

#include "hdf5_config.h"
#include "hdf5_types.h"
#include <IMP/base_types.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <algorithm>
#include <IMP/Pointer.h>


IMPHDF5_BEGIN_NAMESPACE
class HDF5Group;

/** Wrap an HDF5 data set.*/
template <class TypeTraits>
class IMPHDF5EXPORT HDF5DataSet {
  static const int max_dims=3;
  Pointer<HDF5SharedHandle> h_;
  Pointer<HDF5SharedHandle> ids_;
  unsigned int dim_;
  bool get_is_null_value(const Ints &ijk) const {
    return TypeTraits::get_is_null_value(get_value(ijk));
  }
  friend class HDF5Group;
  HDF5DataSet(HDF5SharedHandle* parent, std::string name,  int num_dims) {
    IMP_USAGE_CHECK(num_dims <= max_dims, "Currently it only supports "
                    << max_dims << " dims");
    //std::cout << "Creating data set " << name << std::endl;
    IMP_USAGE_CHECK(!H5Lexists(parent->get_hid(),
                              name.c_str(), H5P_DEFAULT),
                    "Data set " << name << " already exists");
    hsize_t dims[max_dims]={0};
    hsize_t cdims[max_dims]={64};
    if (num_dims >2) {
      std::fill(cdims+1, cdims+num_dims-1, 2);
    }
    if (num_dims >1) {
      cdims[num_dims-1]=1;
    }
    hsize_t maxs[max_dims];
    std::fill(maxs, maxs+num_dims, H5S_UNLIMITED);
    HDF5Handle ds(H5Screate_simple(num_dims, dims, maxs), &H5Sclose);
    HDF5Handle plist(H5Pcreate(H5P_DATASET_CREATE), &H5Pclose);
    IMP_HDF5_CALL(H5Pset_chunk(plist, num_dims, cdims));
    IMP_HDF5_CALL(H5Pset_fill_value(plist, TypeTraits::get_hdf5_type(),
                                    &TypeTraits::get_fill_value()));
    IMP_HDF5_CALL(H5Pset_fill_time(plist, H5D_FILL_TIME_IFSET));
    IMP_HDF5_CALL(H5Pset_alloc_time(plist, H5D_ALLOC_TIME_LATE));
    /*IMP_HDF5_CALL(H5Pset_szip (plist, H5_SZIP_NN_OPTION_MASK,
      32));*/
    IMP_HDF5_CALL(H5Pset_deflate(plist, 9));
    //std::cout << "creating..." << name << std::endl;
    h_= new HDF5SharedHandle(H5Dcreate(parent->get_hid(),
                                       name.c_str(),
                                       TypeTraits::get_hdf5_type(),
                                       ds, H5P_DEFAULT, plist, H5P_DEFAULT),
                             &H5Dclose);
    hsize_t one=1;
    ids_=new HDF5SharedHandle(H5Screate_simple(1, &one, NULL), &H5Sclose);
    dim_=num_dims;
    //std::cout << "done..." << std::endl;
  }
  HDF5DataSet(HDF5SharedHandle* parent, std::string name) {
    IMP_USAGE_CHECK(H5Lexists(parent->get_hid(),
                              name.c_str(), H5P_DEFAULT),
                    "Data set " << name << " does not exist");
    h_= new HDF5SharedHandle(H5Dopen(parent->get_hid(),
                                     name.c_str(), H5P_DEFAULT),
                             &H5Dclose);
    hsize_t one=1;
    ids_=new HDF5SharedHandle(H5Screate_simple(1, &one, NULL), &H5Sclose);
    HDF5Handle s(H5Dget_space(h_->get_hid()), H5Sclose);
    dim_=H5Sget_simple_extent_ndims(s);
  }

 public:
  void show(std::ostream &out) const {
    out << "HDF5DataSet";
  }
  HDF5DataSet(): dim_(0){}
  Ints get_size() const {
    HDF5Handle s(H5Dget_space(h_->get_hid()), H5Sclose);
    hsize_t ret[max_dims];
    IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, ret, NULL));
    return Ints(ret, ret+dim_);
  }
  hid_t get_handle() const {
    return h_->get_hid();
  }

  void set_value(const Ints &ijk,
                 typename TypeTraits::Type value) {
    HDF5Handle s(H5Dget_space(h_->get_hid()), &H5Sclose);
    hsize_t pos[max_dims]; std::copy(ijk.begin(), ijk.end(), pos);
    hsize_t ones[max_dims]; std::fill(ones, ones+dim_, 1);
    IMP_HDF5_CALL(H5Sselect_hyperslab(s, H5S_SELECT_SET, &pos[0],
                                      &ones[0], &ones[0],
                                      NULL));
    TypeTraits::write_value_dataset(h_->get_hid(), ids_->get_hid(),
                                         s, value);
  }
  /*std::vector<typename TypeTraits::Type get_row(unsigned int dim,
                                                unsigned int index) const {
    HDF5Handle s(H5Dget_space(h_->get_hid()), &H5Sclose);
    std::vector<hsize_t> pos(ijk.begin(), ijk.end());
    std::vector<hsize_t> ones(dim_, 1);
    IMP_HDF5_CALL(H5Sselect_hyperslab(s, H5S_SELECT_SET, &pos[0],
                                      &ones[0], &ones[0],
                                      NULL));
    return TypeTraits::read_value_dataset(h_->get_hid(), ids_->get_hid(),
                                              s);
                                              }*/
  typename TypeTraits::Type get_value(const Ints &ijk) const {
    HDF5Handle s(H5Dget_space(h_->get_hid()), &H5Sclose);
    hsize_t pos[max_dims]; std::copy(ijk.begin(), ijk.end(), pos);
    hsize_t ones[max_dims]; std::fill(ones, ones+dim_, 1);
    IMP_HDF5_CALL(H5Sselect_hyperslab(s, H5S_SELECT_SET, &pos[0],
                                      &ones[0], &ones[0],
                                      NULL));
    return TypeTraits::read_value_dataset(h_->get_hid(), ids_->get_hid(),
                                              s);
  }
  void set_size(const Ints &ijk) {
    hsize_t nd[max_dims]; std::copy(ijk.begin(), ijk.end(), nd);;
    IMP_HDF5_CALL(H5Dset_extent(h_->get_hid(),
                                &nd[0]));
  }
};


typedef HDF5DataSet<FloatTraits> HDF5FloatDataSet;
typedef HDF5DataSet<StringTraits> HDF5StringDataSet;
typedef HDF5DataSet<IntTraits> HDF5IntDataSet;
typedef HDF5DataSet<IndexTraits> HDF5IndexDataSet;
typedef HDF5DataSet<NodeIDTraits> HDF5NodeIDDataSet;
typedef HDF5DataSet<DataSetTraits> HDF5DataSetDataSet;
typedef std::vector<HDF5FloatDataSet> HDF5FloatDataSets;
typedef std::vector<HDF5StringDataSet> HDF5StringDataSets;
typedef std::vector<HDF5IntDataSet> HDF5IntDataSets;
typedef std::vector<HDF5IndexDataSet> HDF5IndexDataSets;
typedef std::vector<HDF5NodeIDDataSet> HDF5NodeIDDataSets;
typedef std::vector<HDF5DataSetDataSet> HDF5DataSetDataSets;


/** Wrap an HDF5 Group. */
class IMPHDF5EXPORT HDF5Group {
  Pointer<HDF5SharedHandle> h_;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    IMP_HDF5_CALL(H5Gget_info(h_->get_hid(), &info));
    unsigned int n= info.nlinks;
    return n;
  }
 public:
  void show(std::ostream &out) const {
    out << "HDF5Group";
  }

  // create as root of file
  HDF5Group(std::string name, bool clear);
  // create from an existing group
  HDF5Group(HDF5Group parent, std::string name);
  HDF5Group add_child(std::string name);
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> add_child_data_set(std::string name,
                                    int dim) {
    return HDF5DataSet<TypeTraits>(h_, name, dim);
  }
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> get_child_data_set(std::string name) const {
    return HDF5DataSet<TypeTraits>(h_, name);
  }
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> get_child_data_set(std::string name, int dim) {
    if (get_has_child(name)) {
      return HDF5DataSet<TypeTraits>(h_, name);
    } else {
      return add_child_data_set<TypeTraits>(name, dim);
    }
  }
#define IMP_HDF5_DATA_SET_METHODS(lcname, UCName)                       \
  HDF5DataSet<UCName##Traits> add_child_##lcname##_data_set(std::string name, \
                                                            int dim) {  \
    return add_child_data_set<UCName##Traits>(name, dim);               \
  }                                                                     \
  HDF5DataSet<UCName##Traits> get_child_##lcname##_data_set(std::string name)\
    const {                                                             \
    return get_child_data_set<UCName##Traits>(name);                    \
  }                                                                     \
  HDF5DataSet<UCName##Traits> get_child_##lcname##_data_set(std::string name, \
                                                            int dim) {  \
    return get_child_data_set<UCName##Traits>(name, dim);               \
  }

  IMP_HDF5_DATA_SET_METHODS(int, Int);
  IMP_HDF5_DATA_SET_METHODS(index, Index);
  IMP_HDF5_DATA_SET_METHODS(float, Float);
  IMP_HDF5_DATA_SET_METHODS(string, String);
  IMP_HDF5_DATA_SET_METHODS(data_set, DataSet);
  IMP_HDF5_DATA_SET_METHODS(node_id, NodeID);
  unsigned int get_number_of_children() const;
  std::string get_child_name(unsigned int i) const;
  bool get_has_child(std::string name) const;
  hid_t get_handle() const;
  bool get_child_is_group(unsigned int i) const;
  bool get_child_is_data_set(unsigned int i) const;

  template <class TypeTraits>
    void set_attribute(std::string name,
                       std::vector< typename TypeTraits::Type> value) {
    if (value.empty()) {
      if (H5Aexists(h_->get_hid(), name.c_str())) {
        IMP_HDF5_CALL(H5Adelete(h_->get_hid(), name.c_str()));
      }
    } else {
      bool missing=!H5Aexists(h_->get_hid(), name.c_str());
      if (!missing) {
        hsize_t dim, maxdim;
        {
          HDF5Handle a(H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                       &H5Aclose);
          HDF5Handle s(H5Aget_space(a), &H5Sclose);
          IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
        }
        if (value.size() != dim) {
          IMP_HDF5_CALL(H5Adelete(h_->get_hid(), name.c_str()));
          missing=true;
        }
      }
      if (missing) {
        HDF5Handle s(H5Screate(H5S_SIMPLE), &H5Sclose);
        hsize_t dim=std::max(value.size(), size_t(1));
        hsize_t max=H5S_UNLIMITED;
        IMP_HDF5_CALL(H5Sset_extent_simple(s, 1, &dim, &max));
        HDF5Handle a(H5Acreate2(h_->get_hid(), name.c_str(),
                                TypeTraits::get_hdf5_type(),
                                s, H5P_DEFAULT, H5P_DEFAULT),
                     &H5Aclose);
      }
      HDF5Handle a(H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                   &H5Aclose);
      TypeTraits::write_values_attribute(a, value);
    }
  }
  template <class TypeTraits>
    std::vector<typename TypeTraits::Type>
    get_attribute(std::string name) const {
    if (!H5Aexists(h_->get_hid(), name.c_str())) {
      return std::vector<typename TypeTraits::Type>();
    } else {
      HDF5Handle a(H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                   &H5Aclose);
      HDF5Handle s(H5Aget_space(a), &H5Sclose);
      hsize_t dim, maxdim;
      IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
      std::vector<typename TypeTraits::Type> ret
        = TypeTraits::read_values_attribute(a, dim);
      return ret;
    }
  }
  template <class CT, class CF>
    CT copy_to(const CF &cf) const {
    return CT(cf.begin(), cf.end());
  }
  bool get_has_attribute(std::string nm) const;

#define IMP_HDF5_ATTRIBUTE(lcname, UCName)                              \
  void set_##lcname##_attribute(std::string nm,                         \
                     const std::vector< UCName##Traits::Type> &value) { \
    set_attribute< UCName##Traits>(nm, value);                          \
  }                                                                     \
  std::vector< UCName##Traits::Type>                                    \
    get_##lcname##_attribute(std::string nm) const {                    \
    return get_attribute< UCName##Traits>(nm);                          \
  }                                                                     \

  IMP_HDF5_ATTRIBUTE(float, Float);
  IMP_HDF5_ATTRIBUTE(int, Int);
  IMP_HDF5_ATTRIBUTE(string, String);
  IMP_HDF5_ATTRIBUTE(index, Index);
};


IMP_VALUES(HDF5Group, HDF5Groups);


inline int get_number_of_open_hdf5_handles() {
  H5garbage_collect();
  return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
}


IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_HDF5_WRAPPER_H */
