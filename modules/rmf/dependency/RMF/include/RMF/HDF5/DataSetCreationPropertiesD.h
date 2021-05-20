/**
 *  \file RMF/HDF5/DataSetCreationPropertiesD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_DATA_SET_CREATION_PROPERTIES_D_H
#define RMF_HDF5_DATA_SET_CREATION_PROPERTIES_D_H

#include "RMF/config.h"
#include "DataSetAccessPropertiesD.h"

RMF_ENABLE_WARNINGS namespace RMF {
  namespace HDF5 {

  /** Data sets can be compressed using one of several algorithms.
   */
  enum Compression {
    GZIP_COMPRESSION,
    SLIB_COMPRESSION,
    NO_COMPRESSION
  };

  /** Define properties for creating an HDF5 data set.*/
  template <class TypeTraits, unsigned int D>
  struct DataSetCreationPropertiesD : DataSetAccessPropertiesD<TypeTraits, D> {

    using DataSetAccessPropertiesD<TypeTraits, D>::get_handle;
    void set_compression(Compression comp) {
      if (comp == GZIP_COMPRESSION) {
        RMF_HDF5_CALL(H5Pset_deflate(get_handle(), 9));
      } else if (comp == SLIB_COMPRESSION) {
        RMF_HDF5_CALL(H5Pset_szip(get_handle(), H5_SZIP_NN_OPTION_MASK, 32));
      }
    }
    /** See
       \external{https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetChunk,
       H5Pset_chunk}
     */
    void set_chunk_size(DataSetIndexD<D> chunk_size) {
      hsize_t cdims[D];
      for (unsigned int i = 0; i < D; ++i) {
        cdims[i] = chunk_size[i];
      }
      RMF_HDF5_CALL(H5Pset_chunk(get_handle(), D, cdims));
    }
    DataSetCreationPropertiesD()
        : DataSetAccessPropertiesD<TypeTraits, D>(H5P_DATASET_CREATE) {
      hsize_t cdims[D];
      cdims[0] = 512;
      if (D > 2) {
        std::fill(cdims + 1, cdims + D - 1, 4);
      }
      if (D > 1) {
        cdims[D - 1] = 1;
      }
      RMF_HDF5_CALL(H5Pset_chunk(get_handle(), D, cdims));
      RMF_HDF5_CALL(H5Pset_fill_value(get_handle(),
                                      TypeTraits::get_hdf5_fill_type(),
                                      &TypeTraits::get_fill_value()));
      RMF_HDF5_CALL(H5Pset_fill_time(get_handle(), H5D_FILL_TIME_ALLOC));
      RMF_HDF5_CALL(H5Pset_alloc_time(get_handle(), H5D_ALLOC_TIME_INCR));
    }

    //! Sets custom fill value instead of the default one
    /** Sets custom fill value instead of the default one, which
        is used as a default value for dataset entries that were not
        written explicitly
        (see HDF5 documentation for more details about fill values)

        @param pValue a pointer to the fill value
    */
    void set_custom_fill_value(typename TypeTraits::Type* pValue){
      RMF_HDF5_CALL(H5Pset_fill_value(get_handle(),
                                      TypeTraits::get_hdf5_fill_type(),
                                      pValue)
                    );
    }
  };

  } /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5_DATA_SET_CREATION_PROPERTIES_D_H */
