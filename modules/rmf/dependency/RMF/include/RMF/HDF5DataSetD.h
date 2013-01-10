/**
 *  \file RMF/HDF5DataSetD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5DATA_SET_D_H
#define RMF_HDF_5DATA_SET_D_H

#include <RMF/config.h>
#include "types.h"
#include "HDF5Object.h"
#include "HDF5MutableAttributes.h"
#include "HDF5ConstDataSetD.h"
#include "infrastructure_macros.h"
#include <algorithm>

namespace RMF {

/** Wrap an HDF5 data set. Typedefs and python types are provided for
    data sets in 1,2, and 3 dimensions with all the
    \ref rmf_types "supported types". They are named as
    RMF::HDF5IndexDataSet2D (or RMF.HDF5IndexDataSet2).
   See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame10Datasets.html,
   the HDF5 manual} for more information.
 */
template <class TypeTraits, unsigned int D>
class HDF5DataSetD:
  public HDF5MutableAttributes<HDF5ConstDataSetD<TypeTraits, D> > {
  typedef HDF5MutableAttributes<HDF5ConstDataSetD<TypeTraits, D> >  P;
  friend class HDF5Group;
protected:
  typedef HDF5DataSetCreationPropertiesD<TypeTraits, D> CreationProperties;
  typedef HDF5DataSetAccessPropertiesD<TypeTraits, D> AccessProperties;

  HDF5DataSetD(HDF5SharedHandle* parent, std::string name,
               CreationProperties props):
    P(parent, name, props) {
  }
  HDF5DataSetD(HDF5SharedHandle* parent, std::string name,
               AccessProperties props):
    P(parent, name, props) {
  }
public:
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
  HDF5DataSetD(hid_t file, std::string name,
               AccessProperties props): P(file, name, props) {
  }
#endif
  typedef HDF5DataSetIndexD<D> Index;
  HDF5DataSetD() {
  }

  void set_value(const HDF5DataSetIndexD<D> &ijk,
                 typename TypeTraits::Type  value) {
    RMF_IF_CHECK {
      P::check_index(ijk);
    }
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(P::get_data_space(),
                                      H5S_SELECT_SET, ijk.get(),
                                      P::get_ones(), P::get_ones(),
                                      NULL));
    TypeTraits::write_value_dataset(HDF5Object::get_handle(),
                                    P::get_input_data_space().get_hid(),
                                    P::get_data_space(), value);
  }
  RMF_SHOWABLE(HDF5DataSetD, "HDF5DataSet"
               << D << "D " << P::get_name());
#ifndef SWIG
  typedef HDF5DataSetIndexD<D - 1> RowIndex;
  void set_row( const RowIndex                  & ijkr,
                const typename TypeTraits::Types& value) {
    HDF5DataSetIndexD<D> ijk;
    std::copy(ijkr.begin(), ijkr.end(), ijk.begin());
    ijk[D - 1] = 0;
    RMF_IF_CHECK {
      P::check_index(ijk);
    }
    hsize_t size[D]; std::fill(size, size + D - 1, 1);
    size[D - 1] = P::get_size()[D - 1]; // set last to size of row
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(P::get_data_space(),
                                      H5S_SELECT_SET, ijk.get(),
                                      P::get_ones(), &size[0],
                                      NULL));
    TypeTraits::write_values_dataset(HDF5Object::get_handle(),
                                     P::get_row_data_space().get_hid(),
                                     P::get_data_space(), value);
  }
#endif
  //! Write a rectangular block starting at ln of size size
  void set_block(const Index&lb, const Index &size,
                 const typename TypeTraits::Types& value) {
    RMF_IF_CHECK {
      P::check_index(lb);
      Index last = lb;
      // offset size by one and check...
      unsigned int total = 1;
      for (unsigned int i = 0; i < D; ++i) {
        total *= size[i];
        last[i] += size[i] - 1;
      }
      RMF_USAGE_CHECK(total == value.size(),
                      internal::get_error_message("Block has size ",
                                                  total, " but found ",
                                                  value.size(),
                                                  " values"));
      P::check_index(last);
    }
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(P::get_data_space(),
                                      H5S_SELECT_SET, lb.get(),
                                      P::get_ones(), size.get(),
                                      NULL));
    hsize_t sz = value.size();
    RMF_HDF5_HANDLE(input, H5Screate_simple(1, &sz,
                                            NULL), &H5Sclose);
    TypeTraits::write_values_dataset(HDF5Object::get_handle(),
                                     input,
                                     P::get_data_space(), value);
  }
  void set_size(const HDF5DataSetIndexD<D> &ijk) {
    hsize_t nd[D]; std::copy(ijk.begin(), ijk.end(), nd);;
    RMF_HDF5_CALL(H5Dset_extent(HDF5Object::get_handle(),
                                &nd[0]));
    P::initialize_handles();
  }
};



#ifndef RMF_DOXYGEN

#  define RMF_DECLARE_DATA_SET(lcname, Ucname, PassValue, ReturnValue, \
                               PassValues, ReturnValues)               \
  typedef HDF5DataSetD<Ucname##Traits, 1> HDF5##Ucname##DataSet1D;     \
  typedef vector<HDF5##Ucname##DataSet1D> HDF5##Ucname##DataSet1Ds;    \
  typedef HDF5DataSetD<Ucname##Traits, 2> HDF5##Ucname##DataSet2D;     \
  typedef vector<HDF5##Ucname##DataSet2D> HDF5##Ucname##DataSet2Ds;    \
  typedef HDF5DataSetD<Ucname##Traits, 3> HDF5##Ucname##DataSet3D;     \
  typedef vector<HDF5##Ucname##DataSet3D> HDF5##Ucname##DataSet3Ds;    \
  typedef HDF5MutableAttributes<HDF5##Ucname##ConstDataSet1D>          \
    HDF5##Ucname##DataSet1DAttributes;                                 \
  typedef HDF5MutableAttributes<HDF5##Ucname##ConstDataSet2D>          \
    HDF5##Ucname##DataSet2DAttributes;                                 \
  typedef HDF5MutableAttributes<HDF5##Ucname##ConstDataSet3D>          \
    HDF5##Ucname##DataSet3DAttributes;                                 \
  typedef vector<HDF5##Ucname##DataSet1DAttributes>                    \
    HDF5##Ucname##DataSet1DAttributesList;                             \
  typedef vector<HDF5##Ucname##DataSet2DAttributes>                    \
    HDF5##Ucname##DataSet2DAttributesList;                             \
  typedef vector<HDF5##Ucname##DataSet3DAttributes>                    \
    HDF5##Ucname##DataSet3DAttributesList


/** \name Basic data set types
     \ingroup hdf5
     @{
 */
RMF_FOREACH_TYPE(RMF_DECLARE_DATA_SET);
/** @} */
#endif
} /* namespace RMF */

#endif /* RMF_HDF_5DATA_SET_D_H */
