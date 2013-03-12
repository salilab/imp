/**
 *  \file RMF/DataSetD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5DATA_SET_D_H
#define RMF_HDF_5DATA_SET_D_H

#include <RMF/config.h>
#include "types.h"
#include "Object.h"
#include "MutableAttributes.h"
#include "ConstDataSetD.h"
#include "infrastructure_macros.h"
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

/** Wrap an HDF5 data set. Typedefs and python types are provided for
    data sets in 1,2, and 3 dimensions with all the
    \ref rmf_types "supported types". They are named as
    RMF::HDF5::IndexDataSet2D (or RMF.HDF5.IndexDataSet2).
   See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame10Datasets.html,
   the HDF5 manual} for more information.
 */
template <class TypeTraits, unsigned int D>
class DataSetD:
  public MutableAttributes<ConstDataSetD<TypeTraits, D> > {
  typedef MutableAttributes<ConstDataSetD<TypeTraits, D> >  P;
  friend class Group;
protected:
  typedef DataSetCreationPropertiesD<TypeTraits, D> CreationProperties;
  typedef DataSetAccessPropertiesD<TypeTraits, D> AccessProperties;

  DataSetD(SharedHandle* parent, std::string name,
               CreationProperties props):
    P(parent, name, props) {
  }
  DataSetD(SharedHandle* parent, std::string name,
               AccessProperties props):
    P(parent, name, props) {
  }
public:
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
  DataSetD(hid_t file, std::string name,
               AccessProperties props): P(file, name, props) {
  }
#endif
  typedef DataSetIndexD<D> Index;
  DataSetD() {
  }

  void set_value(const DataSetIndexD<D> &ijk,
                 typename TypeTraits::Type  value) {
    RMF_IF_CHECK {
      P::check_index(ijk);
    }
    //RMF_HDF5_HANDLE(sel, H5Dget_space(h_->get_hid()), &H5Sclose);
    RMF_HDF5_CALL(H5Sselect_hyperslab(P::get_data_space(),
                                      H5S_SELECT_SET, ijk.get(),
                                      P::get_ones(), P::get_ones(),
                                      NULL));
    TypeTraits::write_value_dataset(Object::get_handle(),
                                    P::get_input_data_space().get_hid(),
                                    P::get_data_space(), value);
  }
  RMF_SHOWABLE(DataSetD, "DataSet"
               << D << "D " << P::get_name());
#ifndef SWIG
  typedef DataSetIndexD<D - 1> RowIndex;
  void set_row( const RowIndex                  & ijkr,
                const typename TypeTraits::Types& value) {
    DataSetIndexD<D> ijk;
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
    TypeTraits::write_values_dataset(Object::get_handle(),
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
                      RMF::internal::get_error_message("Block has size ",
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
    TypeTraits::write_values_dataset(Object::get_handle(),
                                     input,
                                     P::get_data_space(), value);
  }
  void set_size(const DataSetIndexD<D> &ijk) {
    hsize_t nd[D]; std::copy(ijk.begin(), ijk.end(), nd);;
    RMF_HDF5_CALL(H5Dset_extent(Object::get_handle(),
                                &nd[0]));
    P::initialize_handles();
  }
};



#ifndef RMF_DOXYGEN

#  define RMF_DECLARE_DATA_SET(lcname, Ucname, PassValue, ReturnValue, \
                               PassValues, ReturnValues)               \
  typedef DataSetD<Ucname##Traits, 1> Ucname##DataSet1D;     \
  typedef std::vector<Ucname##DataSet1D> Ucname##DataSet1Ds;    \
  typedef DataSetD<Ucname##Traits, 2> Ucname##DataSet2D;     \
  typedef std::vector<Ucname##DataSet2D> Ucname##DataSet2Ds;    \
  typedef DataSetD<Ucname##Traits, 3> Ucname##DataSet3D;     \
  typedef std::vector<Ucname##DataSet3D> Ucname##DataSet3Ds;    \
  typedef MutableAttributes<Ucname##DataSet1D>                   \
    Ucname##DataSet1DAttributes;                                 \
  typedef MutableAttributes<Ucname##DataSet2D>                   \
    Ucname##DataSet2DAttributes;                                 \
  typedef MutableAttributes<Ucname##DataSet3D>                   \
    Ucname##DataSet3DAttributes;                                 \
  typedef std::vector<Ucname##DataSet1DAttributes>                    \
    Ucname##DataSet1DAttributesList;                             \
  typedef std::vector<Ucname##DataSet2DAttributes>                    \
    Ucname##DataSet2DAttributesList;                             \
  typedef std::vector<Ucname##DataSet3DAttributes>                    \
    Ucname##DataSet3DAttributesList


/** \name Basic data set types
     \ingroup hdf5
     @{
 */
RMF_FOREACH_HDF5_TYPE(RMF_DECLARE_DATA_SET);
/** @} */
#endif

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5DATA_SET_D_H */
