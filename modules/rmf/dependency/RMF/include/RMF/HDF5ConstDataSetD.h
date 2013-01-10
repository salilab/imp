/**
 *  \file RMF/HDF5ConstDataSetD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5CONST_DATA_SET_D_H
#define RMF_HDF_5CONST_DATA_SET_D_H

#include <RMF/config.h>
#include "types.h"
#include "HDF5ConstAttributes.h"
#include "HDF5Object.h"
#include "HDF5DataSetIndexD.h"
#include "HDF5DataSetCreationPropertiesD.h"
#include "infrastructure_macros.h"
#include <algorithm>
#include <boost/shared_ptr.hpp>

namespace RMF {
class HDF5Group;
typedef HDF5ConstAttributes<HDF5Object> HDF5ConstDataSetAttributes;
#ifndef RMF_DOXYGEN
typedef vector<HDF5ConstDataSetAttributes> HDF5ConstDataSetAttributesList;
#endif


/** Wrap an HDF5 data set. Typedefs and python types are provided for
    data sets in 1,2, and 3 dimensions with all the
    \ref rmf_types "supported types". They are named as
    RMF::HDF5IndexDataSet2D (or RMF.HDF5IndexDataSet2).
   See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame10Datasets.html,
   the HDF5 manual} for more information.
 */
template <class TypeTraits, unsigned int D>
class HDF5ConstDataSetD: public HDF5ConstDataSetAttributes {
  typedef HDF5ConstDataSetAttributes P;
  struct Data {
    HDF5Handle ids_;
    HDF5Handle rds_;
    HDF5Handle sel_;
    hsize_t ones_[D];
    HDF5DataSetIndexD<D> size_;
  };

  boost::shared_ptr<Data> data_;
  int compare(const HDF5ConstDataSetD<TypeTraits, D> &o) const {
    // not great, but...
    if (data_ && !o.data_) return -1;
    else if (o.data_ && !data_) return 1;
    else if (!o.data_ && !data_) return 0;
    else if (get_name() < o.get_name()) return -1;
    else if (get_name() > o.get_name()) return 1;
    else return 0;
  }

  bool get_is_null_value(const HDF5DataSetIndexD<D> &ijk) const {
    return TypeTraits::get_is_null_value(get_value(ijk));
  }
  void initialize() {
    hsize_t one = 1;
    data_->ids_.open(H5Screate_simple(1, &one, NULL), &H5Sclose);
    std::fill(data_->ones_, data_->ones_ + D, 1);
    //pos_.reset(new hsize_t[dim_]);
    //sel_= new HDF5SharedHandle(H5Dget_space(h_->get_hid()), &H5Sclose);
    initialize_handles();
  }
  friend class HDF5ConstGroup;
protected:
  typedef HDF5DataSetCreationPropertiesD<TypeTraits, D> CreationProperties;
  typedef HDF5DataSetAccessPropertiesD<TypeTraits, D> AccessProperties;

  HDF5ConstDataSetD(HDF5SharedHandle* parent, std::string name,
                    CreationProperties props):
    data_(new Data()) {
    //std::cout << "Creating data set " << name << std::endl;
    RMF_USAGE_CHECK(!H5Lexists(parent->get_hid(),
                               name.c_str(), H5P_DEFAULT),
                    internal::get_error_message("Data set ", name,
                                                " already exists"));
    hsize_t dims[D] = {0};
    hsize_t maxs[D];
    std::fill(maxs, maxs + D, H5S_UNLIMITED);
    RMF_HDF5_HANDLE(ds, H5Screate_simple(D, dims, maxs), &H5Sclose);
    //std::cout << "creating..." << name << std::endl;
    P::open(new HDF5SharedHandle(H5Dcreate2(parent->get_hid(),
                                            name.c_str(),
                                            TypeTraits::get_hdf5_disk_type(),
                                            ds, H5P_DEFAULT,
                                            props.get_handle(), H5P_DEFAULT),
                                 &H5Dclose, name));
    initialize();
    //std::cout << "done..." << std::endl;
  }
  HDF5ConstDataSetD(HDF5SharedHandle* parent,
                    std::string name, AccessProperties props):
    data_(new Data()) {
    RMF_USAGE_CHECK(H5Lexists(parent->get_hid(),
                              name.c_str(), H5P_DEFAULT),
                    internal::get_error_message("Data set ",
                                                name,
                                                " does not exist"));
    P::open(new HDF5SharedHandle(H5Dopen2(parent->get_hid(),
                                          name.c_str(), props.get_handle()),
                                 &H5Dclose, name));
    //RMF_HDF5_HANDLE(s, H5Dget_space(h_->get_hid()), H5Sclose);
    RMF_HDF5_HANDLE(sel, H5Dget_space(HDF5Object::get_handle()), &H5Sclose);
    RMF_USAGE_CHECK(H5Sget_simple_extent_ndims(sel) == D,
                    internal::get_error_message(
                      "Dimensions don't match. Got ",
                      H5Sget_simple_extent_ndims(sel),
                      " but expected ", D));
    initialize();
  }
  hsize_t* get_ones() const {
    return data_->ones_;
  }
  const HDF5Handle& get_row_data_space() const {
    return data_->rds_;
  }
  const HDF5Handle& get_data_space() const {
    return data_->sel_;
  }
  const HDF5Handle& get_input_data_space() const {
    return data_->ids_;
  }
  void check_index(const HDF5DataSetIndexD<D> &ijk) const {
    HDF5DataSetIndexD<D> sz = get_size();
    for (unsigned int i = 0; i < D; ++i) {
      RMF_USAGE_CHECK(ijk[i] < sz[i],
                      internal::get_error_message("Index is out of range: "
                                                  , ijk[i], " >= ", sz[i]));
    }
  }
  void initialize_handles() {
    data_->sel_.open(H5Dget_space(HDF5Object::get_handle()), &H5Sclose);
    // must be second
    hsize_t ret[D];
    std::fill(ret, ret + D, -1);
    RMF_HDF5_CALL(H5Sget_simple_extent_dims(get_data_space(),
                                            ret, NULL));
    RMF_INTERNAL_CHECK(ret[D - 1] < 1000000,
                       "extents not returned properly");
    if (ret[D - 1] > 0) {
      // some versions will spew an error on this
      // we will call this function again before rds_ is needed
      //std::cout << "inializing row to " << ret[data_->dim_-1] << std::endl;
      data_->rds_.open(H5Screate_simple(1, ret + D - 1,
                                        NULL), &H5Sclose);
    } else {
      //std::cout << "clearing row data" << std::endl;
      data_->rds_.close();
    }
    RMF_HDF5_CALL(H5Sget_simple_extent_dims(get_data_space(),
                                            data_->size_.begin(), NULL));
  }
public:
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
  HDF5ConstDataSetD(hid_t file, std::string name): data_(new Data()) {
    RMF_USAGE_CHECK(H5Lexists(file,
                              name.c_str(), H5P_DEFAULT),
                    internal::get_error_message("Data set ", name,
                                                " does not exist"));
    P::open(new HDF5SharedHandle(H5Dopen2(file,
                                          name.c_str(), H5P_DEFAULT),
                                 &H5Dclose, name));
    //RMF_HDF5_HANDLE(s, H5Dget_space(h_->get_hid()), H5Sclose);
    RMF_HDF5_HANDLE(sel, H5Dget_space(HDF5Object::get_handle()), &H5Sclose);
    RMF_USAGE_CHECK(H5Sget_simple_extent_ndims(sel) == D,
                    internal::get_error_message("Dimensions don't match. Got ",
                                                H5Sget_simple_extent_ndims(sel),
                                                " but expected ", D));
    initialize();
  }
#endif
  typedef HDF5DataSetIndexD<D> Index;
  HDF5ConstDataSetD() {
  }
  HDF5DataSetIndexD<D> get_size() const {
    //RMF_HDF5_HANDLE(s, H5Dget_space(h_->get_hid()), H5Sclose);
    return data_->size_;
  }
  typename TypeTraits::Type get_value(const HDF5DataSetIndexD<D> &ijk) const {
    RMF_IF_CHECK {
      check_index(ijk);
    }
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, ijk.get(),
                                      data_->ones_, data_->ones_,
                                      NULL));
    return TypeTraits::read_value_dataset(HDF5Object::get_handle(),
                                          data_->ids_.get_hid(),
                                          get_data_space());
  }
  RMF_SHOWABLE(HDF5ConstDataSetD, "HDF5ConstDataSet"
               << D << "D " << P::get_name());
#ifndef SWIG
  typedef HDF5DataSetIndexD<D - 1> RowIndex;
  typename TypeTraits::Types get_row( const RowIndex ijkr) const {
    HDF5DataSetIndexD<D> ijk;
    std::copy(ijkr.begin(), ijkr.end(), ijk.begin());
    ijk[D - 1] = 0;
    RMF_IF_CHECK {
      check_index(ijk);
    }
    hsize_t size[D]; std::fill(size, size + D - 1, 1);
    size[D - 1] = get_size()[D - 1]; // set last to size of row
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, ijk.get(),
                                      data_->ones_, &size[0],
                                      NULL));
    return TypeTraits::read_values_dataset(HDF5Object::get_handle(),
                                           get_row_data_space().get_hid(),
                                           get_data_space(),
                                           size[D - 1]);
  }
#endif
  //! Read a rectangular block starting at ln of size size
  typename TypeTraits::Types get_block( const Index &lb,
                                        const Index &size) const {
    hsize_t total = 1;
    for (unsigned int i = 0; i < D; ++i) {
      total *= size[i];
    }
    RMF_IF_CHECK {
      check_index(lb);
    }
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(get_data_space(),
                                      H5S_SELECT_SET, lb.get(),
                                      data_->ones_, size.get(),
                                      NULL));
    RMF_HDF5_HANDLE(input, H5Screate_simple(1, &total,
                                            NULL), &H5Sclose);
    typename TypeTraits::Types ret
      = TypeTraits::read_values_dataset(HDF5Object::get_handle(),
                                        input,
                                        get_data_space(),
                                        total);
    RMF_INTERNAL_CHECK(ret.size() == total,
                       "Size mismatch");
    return ret;
  }
  RMF_COMPARISONS(HDF5ConstDataSetD);
};



#ifndef RMF_DOXYGEN
#  define RMF_DECLARE_CONST_DATA_SET(lcname, Ucname, PassValue, ReturnValue,  \
                                     PassValues, ReturnValues)                \
  typedef HDF5ConstDataSetD<Ucname##Traits, 1> HDF5##Ucname##ConstDataSet1D;  \
  typedef vector<HDF5##Ucname##ConstDataSet1D> HDF5##Ucname##ConstDataSet1Ds; \
  typedef HDF5ConstDataSetD<Ucname##Traits, 2> HDF5##Ucname##ConstDataSet2D;  \
  typedef vector<HDF5##Ucname##ConstDataSet2D> HDF5##Ucname##ConstDataSet2Ds; \
  typedef HDF5ConstDataSetD<Ucname##Traits, 3> HDF5##Ucname##ConstDataSet3D;  \
  typedef vector<HDF5##Ucname##ConstDataSet3D> HDF5##Ucname##ConstDataSet3Ds

/** \name Basic data set types
     \ingroup hdf5
     @{
 */
RMF_FOREACH_TYPE(RMF_DECLARE_CONST_DATA_SET);
/** @} */
#endif
} /* namespace RMF */

#endif /* RMF_HDF_5CONST_DATA_SET_D_H */
