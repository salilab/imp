/**
 *  \file IMP/rmf/HDF5DataSet.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_HDF_5DATA_SET_H
#define IMPRMF_HDF_5DATA_SET_H

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
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>


namespace rmf {
class HDF5Group;

  /** Store an index into a data set.*/
  template <int D>
  class HDF5DataSetIndex
  {
    int d_[D];
    int compare(const HDF5DataSetIndex<D> &o) const {
      for (unsigned int i=0; i< D; ++i) {
        if (d_[i] < o.d_[i]) return -1;
        else if (d_[i] > o.d_[i]) return 1;
      }
      return 0;
    }
  public:
    HDF5DataSetIndex() {
      std::fill(d_, d_+D, -1);
    }
    HDF5DataSetIndex(unsigned int i) {
      IMP_RMF_USAGE_CHECK(D==1, "Constructor does not match dimension.");
      d_[0]=i;
    }
    HDF5DataSetIndex(unsigned int i, unsigned int j) {
      IMP_RMF_USAGE_CHECK(D==2, "Constructor does not match dimension.");
      d_[0]=i;
      d_[1]=j;
    }
    HDF5DataSetIndex(unsigned int i, unsigned int j, unsigned int k) {
      IMP_RMF_USAGE_CHECK(D==3, "Constructor does not match dimension.");
      d_[0]=i;
      d_[1]=j;
      d_[2]=k;
    }
#ifndef SWIG
    int& operator[](unsigned int i) {
      IMP_RMF_USAGE_CHECK(i < D, "Index out of range: "<< i);
      return d_[i];
    }
    int operator[](unsigned int i) const {
      IMP_RMF_USAGE_CHECK(i < D, "Index out of range: "<< i);
      return d_[i];
    }
#endif
    int __getitem__(unsigned int i) const {
      if (!i > D) {
        IMP_RMF_THROW("Bad index " << i, std::runtime_error);
      }
      return operator[](i);
    }

    unsigned int get_dimension() const {return D;}

    IMP_RMF_COMPARISONS(HDF5DataSetIndex);
    IMP_RMF_HASHABLE(HDF5DataSetIndex,
                     return boost::hash_range(d_, d_+D));
  };
#ifndef SWIG
  template <int D>
  std::ostream &operator<<(std::ostream &out,
                           const HDF5DataSetIndex<D> &id) {
    out << "(";
    for (unsigned int i=0; i< D; ++i) {
      if (i > 0) {
        out << ", ";
      }
      out << id[i];
    }
    out << ")";
    return out;
  }
#endif


/** Wrap an HDF5 data set.*/
template <class TypeTraits>
class HDF5DataSet {
  static const int max_dims=3;
  struct Data {
    HDF5Handle h_;
    HDF5Handle ids_;
    HDF5Handle rds_;
    HDF5Handle sel_;
    hsize_t ones_[max_dims];
    mutable hsize_t pos_[max_dims];
    unsigned int dim_;
  };
  boost::shared_ptr<Data> data_;

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

#define IMP_RMF_DECLARE_DATA_SET(lcname, Ucname, PassValue, ReturnValue,\
                                 PassValues, ReturnValues)              \
  typedef HDF5DataSet<Ucname##Traits> HDF5##Ucname##DataSet;            \
  typedef std::vector<HDF5##Ucname##DataSet> HDF5##Ucname##DataSets

IMP_RMF_FOREACH_TYPE(IMP_RMF_DECLARE_DATA_SET);

} /* namespace rmf */

#endif /* IMPRMF_HDF_5DATA_SET_H */
