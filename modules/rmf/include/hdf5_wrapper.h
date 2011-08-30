/**
 *  \file IMP/rmf/hdf5_wrapper.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_HDF_5_WRAPPER_H
#define IMPRMF_HDF_5_WRAPPER_H

#include "rmf_config.h"
#include "hdf5_types.h"
#include "hdf5_handle.h"
#include "infrastructure_macros.h"
#include <IMP/base_types.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <algorithm>
#include <IMP/Pointer.h>
#include <boost/shared_array.hpp>
#include <boost/intrusive_ptr.hpp>


IMPRMF_BEGIN_NAMESPACE
class HDF5Group;

/** Wrap an HDF5 data set.*/
template <class TypeTraits>
class HDF5DataSet {
  static const int max_dims=3;
  struct Data: public RefCounted {
    HDF5Handle h_;
    HDF5Handle ids_;
    HDF5Handle rds_;
    HDF5Handle sel_;
    hsize_t ones_[max_dims];
    mutable hsize_t pos_[max_dims];
    unsigned int dim_;
  };
  Pointer<Data> data_;
  bool get_is_null_value(const Ints &ijk) const {
    return TypeTraits::get_is_null_value(get_value(ijk));
  }
  const HDF5Handle& get_row_data_space() const {
    return data_->rds_;
  }
  const HDF5Handle& get_data_space() const {
    return data_->sel_;
  }
  void initialize_handles() {
    data_->sel_.open(H5Dget_space(data_->h_.get_hid()), &H5Sclose);
    // must be second
    hsize_t ret[max_dims]={-1,-1,-1};
    IMP_HDF5_CALL(H5Sget_simple_extent_dims(get_data_space(),
                                            ret, NULL));
    IMP_RMF_INTERNAL_CHECK(ret[data_->dim_-1] <1000000,
                       "extents not returned properly");
    if (ret[data_->dim_-1] > 0) {
      // some versions will spew an error on this
      // we will call this function again before rds_ is needed
      //std::cout << "inializing row to " << ret[data_->dim_-1] << std::endl;
      data_->rds_.open(H5Screate_simple(1, ret+data_->dim_-1,
                                        NULL), &H5Sclose);
    } else {
      //std::cout << "clearing row data" << std::endl;
      data_->rds_.close();
    }
  }
  void initialize() {
    hsize_t one=1;
    data_->ids_.open(H5Screate_simple(1, &one, NULL), &H5Sclose);
    std::fill(data_->ones_, data_->ones_+data_->dim_, 1);
    //pos_.reset(new hsize_t[dim_]);
    //sel_= new HDF5SharedHandle(H5Dget_space(h_->get_hid()), &H5Sclose);
    initialize_handles();
  }
  friend class HDF5Group;
  HDF5DataSet(HDF5SharedHandle* parent, std::string name,  int num_dims):
    data_(new Data()) {
    IMP_RMF_USAGE_CHECK(num_dims <= max_dims, "Currently it only supports "
                    << max_dims << " dims");
    //std::cout << "Creating data set " << name << std::endl;
    IMP_RMF_USAGE_CHECK(!H5Lexists(parent->get_hid(),
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
    IMP_HDF5_CALL(H5Pset_fill_value(plist, TypeTraits::get_hdf5_fill_type(),
                                    &TypeTraits::get_fill_value()));
    IMP_HDF5_CALL(H5Pset_fill_time(plist, H5D_FILL_TIME_IFSET));
    IMP_HDF5_CALL(H5Pset_alloc_time(plist, H5D_ALLOC_TIME_LATE));
    /*IMP_HDF5_CALL(H5Pset_szip (plist, H5_SZIP_NN_OPTION_MASK,
      32));*/
    IMP_HDF5_CALL(H5Pset_deflate(plist, 9));
    //std::cout << "creating..." << name << std::endl;
    data_->h_.open(H5Dcreate(parent->get_hid(),
                             name.c_str(),
                             TypeTraits::get_hdf5_disk_type(),
                             ds, H5P_DEFAULT, plist, H5P_DEFAULT),
                   &H5Dclose);
    data_->dim_=num_dims;
    initialize();
    //std::cout << "done..." << std::endl;
  }
  HDF5DataSet(HDF5SharedHandle* parent, std::string name): data_(new Data()) {
    IMP_RMF_USAGE_CHECK(H5Lexists(parent->get_hid(),
                              name.c_str(), H5P_DEFAULT),
                    "Data set " << name << " does not exist");
    data_->h_.open(H5Dopen(parent->get_hid(),
                           name.c_str(), H5P_DEFAULT),
                   &H5Dclose);
    //HDF5Handle s(H5Dget_space(h_->get_hid()), H5Sclose);
    HDF5Handle sel(H5Dget_space(data_->h_.get_hid()), &H5Sclose);
    data_->dim_=H5Sget_simple_extent_ndims(sel);
    initialize();
  }
  void check_index(const Ints &ijk) const {
    Ints sz= get_size();
    IMP_RMF_USAGE_CHECK(ijk.size()==sz.size(), "Index dimensions don't match: "
                    << sz.size() << " != " << ijk.size());
    for (unsigned int i=0; i< sz.size(); ++i) {
      IMP_RMF_USAGE_CHECK(ijk[i] >=0, "Index is negative: " << ijk[i]);
      IMP_RMF_USAGE_CHECK(ijk[i] < sz[i], "Index is out of range: "
                      << ijk[i] << " >= " << sz[i]);
    }
  }
 public:
  void show(std::ostream &out) const {
    out << "HDF5DataSet";
  }
  HDF5DataSet(){}
  Ints get_size() const {
    //HDF5Handle s(H5Dget_space(h_->get_hid()), H5Sclose);
    hsize_t ret[max_dims];
    IMP_HDF5_CALL(H5Sget_simple_extent_dims(get_data_space(),
                                            ret, NULL));
    return Ints(ret, ret+data_->dim_);
  }
  hid_t get_handle() const {
    return data_->h_.get_hid();
  }

  void set_value(const Ints &ijk,
                 typename TypeTraits::Type value) {
    IMP_RMF_IF_CHECK {
      check_index(ijk);
    }
    std::copy(ijk.begin(), ijk.end(), data_->pos_);
    //HDF5Handle sel(H5Dget_space(h_->get_hid()), &H5Sclose);
    IMP_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, data_->pos_,
                                      data_->ones_, data_->ones_,
                                      NULL));
    TypeTraits::write_value_dataset(data_->h_.get_hid(), data_->ids_.get_hid(),
                                    get_data_space(), value);
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
    IMP_RMF_IF_CHECK {
      check_index(ijk);
    }
    std::copy(ijk.begin(), ijk.end(), data_->pos_);
    //HDF5Handle sel(H5Dget_space(h_->get_hid()), &H5Sclose);
    IMP_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, data_->pos_,
                                      data_->ones_, data_->ones_,
                                      NULL));
    return TypeTraits::read_value_dataset(data_->h_.get_hid(),
                                          data_->ids_.get_hid(),
                                          get_data_space());
  }
  void set_row( Ints ijk,
               const typename TypeTraits::Types& value) {
    ijk.push_back(0);
    IMP_RMF_IF_CHECK {
      check_index(ijk);
    }
    std::copy(ijk.begin(), ijk.end(), data_->pos_);
    hsize_t size[max_dims]; std::fill(size, size+data_->dim_-1, 1);
    size[data_->dim_-1]= get_size().back(); // set last to size of row
    //HDF5Handle sel(H5Dget_space(h_->get_hid()), &H5Sclose);
    IMP_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, data_->pos_,
                                      data_->ones_, &size[0],
                                      NULL));
    TypeTraits::write_values_dataset(data_->h_.get_hid(),
                                     get_row_data_space().get_hid(),
                                     get_data_space(), value);
  }
  typename TypeTraits::Types get_row( Ints ijk) const {
    ijk.push_back(0);
    IMP_RMF_IF_CHECK {
      check_index(ijk);
    }
    std::copy(ijk.begin(), ijk.end(), data_->pos_);
    hsize_t size[max_dims]; std::fill(size, size+data_->dim_-1, 1);
    size[data_->dim_-1]= get_size().back(); // set last to size of row
    //HDF5Handle sel(H5Dget_space(h_->get_hid()), &H5Sclose);
    IMP_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, data_->pos_,
                                      data_->ones_, &size[0],
                                      NULL));
    return TypeTraits::read_values_dataset(data_->h_.get_hid(),
                                           get_row_data_space().get_hid(),
                                           get_data_space(),
                                           size[data_->dim_-1]);
  }
  void set_size(const Ints &ijk) {
    hsize_t nd[max_dims]; std::copy(ijk.begin(), ijk.end(), nd);;
    IMP_HDF5_CALL(H5Dset_extent(data_->h_.get_hid(),
                                &nd[0]));
    initialize_handles();
  }
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // replace with safe bool
  operator bool() const {
    return data_;
  }
  bool operator!() const {
    return !data_;
  }
#endif
};

#define IMP_RMF_DECLARE_DATA_SET(lcname, Ucname)                        \
  typedef HDF5DataSet<Ucname##Traits> HDF5##Ucname##DataSet;            \
  typedef std::vector<HDF5##Ucname##DataSet> HDF5##Ucname##DataSets

IMP_RMF_FOREACH_TYPE(IMP_RMF_DECLARE_DATA_SET);

/** Wrap an HDF5 Group. */
class IMPRMFEXPORT HDF5Group {
  boost::intrusive_ptr<HDF5SharedHandle> h_;
  unsigned int get_number_of_links() const {
    H5G_info_t info;
    IMP_HDF5_CALL(H5Gget_info(h_->get_hid(), &info));
    unsigned int n= info.nlinks;
    return n;
  }
#ifndef SWIG
 protected:
  HDF5Group(HDF5SharedHandle *h);
#endif
 public:
  void show(std::ostream &out) const {
    out << "HDF5Group";
  }

  // create from an existing group
  HDF5Group(HDF5Group parent, std::string name);
  HDF5Group add_child(std::string name);
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> add_child_data_set(std::string name,
                                    int dim) {
    return HDF5DataSet<TypeTraits>(h_.get(), name, dim);
  }
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> get_child_data_set(std::string name) const {
    return HDF5DataSet<TypeTraits>(h_.get(), name);
  }
  template <class TypeTraits>
    HDF5DataSet<TypeTraits> get_child_data_set(std::string name, int dim) {
    if (get_has_child(name)) {
      return HDF5DataSet<TypeTraits>(h_.get(), name);
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
  IMP_RMF_FOREACH_TYPE(IMP_HDF5_DATA_SET_METHODS);

  unsigned int get_number_of_children() const;
  std::string get_child_name(unsigned int i) const;
  bool get_has_child(std::string name) const;
  hid_t get_handle() const;
  bool get_child_is_group(unsigned int i) const;
  bool get_child_is_data_set(unsigned int i) const;

  template <class TypeTraits>
    void set_attribute(std::string name,
                       typename TypeTraits::Types value) {
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
                                TypeTraits::get_hdf5_disk_type(),
                                s, H5P_DEFAULT, H5P_DEFAULT),
                     &H5Aclose);
      }
      HDF5Handle a(H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                   &H5Aclose);
      TypeTraits::write_values_attribute(a, value);
    }
  }
  template <class TypeTraits>
    typename TypeTraits::Types
    get_attribute(std::string name) const {
    if (!H5Aexists(h_->get_hid(), name.c_str())) {
      return typename TypeTraits::Types();
    } else {
      HDF5Handle a(H5Aopen(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                   &H5Aclose);
      HDF5Handle s(H5Aget_space(a), &H5Sclose);
      hsize_t dim, maxdim;
      IMP_HDF5_CALL(H5Sget_simple_extent_dims(s, &dim, &maxdim));
      typename TypeTraits::Types ret
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
                                const UCName##Traits::Types &value) {   \
    set_attribute< UCName##Traits>(nm, value);                          \
  }                                                                     \
  UCName##Traits::Types                                                 \
    get_##lcname##_attribute(std::string nm) const {                    \
    return get_attribute< UCName##Traits>(nm);                          \
  }                                                                     \

  IMP_RMF_FOREACH_SIMPLE_TYPE(IMP_HDF5_ATTRIBUTE);
  IMP_HDF5_ATTRIBUTE(char, Char);
};


class IMPRMFEXPORT HDF5File: public HDF5Group {
 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  HDF5File(HDF5SharedHandle *h);
#endif
  void flush();
  ~HDF5File();
};

/** Create a new hdf5 file, clearing any existing file with the same
    name if needed.
*/
IMPRMFEXPORT HDF5File create_hdf5_file(std::string name);

/** Open an existing hdf5 file.
*/
IMPRMFEXPORT HDF5File open_hdf5_file(std::string name);

/** Open an existing hdf5 file.
*/
IMPRMFEXPORT HDF5File open_hdf5_file_read_only(std::string name);

IMP_VALUES(HDF5Group, HDF5Groups);
IMP_VALUES(HDF5File, HDF5Files);


inline int get_number_of_open_hdf5_handles() {
  H5garbage_collect();
  return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
}


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_HDF_5_WRAPPER_H */
