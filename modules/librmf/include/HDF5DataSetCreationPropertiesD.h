/**
 *  \file RMF/HDF5DataSetCreationPropertiesD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5DATA_SET_CREATION_PROPERTIES_D_H
#define IMPLIBRMF_HDF_5DATA_SET_CREATION_PROPERTIES_D_H

#include "RMF_config.h"
#include "HDF5DataSetAccessPropertiesD.h"


namespace RMF {
/** Data sets can be compressed using one of several algorithms.
 */
enum Compression {GZIP_COMPRESSION, SLIB_COMPRESSION, NO_COMPRESSION};

/** Define properties for creating an HDF5 data set.*/
template <class TypeTraits, unsigned int D>
struct HDF5DataSetCreationPropertiesD:
      HDF5DataSetAccessPropertiesD<TypeTraits, D> {
  using HDF5DataSetAccessPropertiesD<TypeTraits, D>::get_handle;
  void set_compression(Compression comp) {
    if (comp == GZIP_COMPRESSION) {
      IMP_HDF5_CALL(H5Pset_deflate(get_handle(), 9));
    } else if (comp == SLIB_COMPRESSION) {
      IMP_HDF5_CALL(H5Pset_szip (get_handle(), H5_SZIP_NN_OPTION_MASK,
                                 32));
    }
  }
  /** See
    \external{http://www.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetChunk, H5Pset_chunk}
   */
  void set_chunk_size(HDF5DataSetIndexD<D> chunk_size) {
    hsize_t cdims[D];
    for (unsigned int i=0; i< D; ++i) {
      cdims[i]=chunk_size[i];
    }
    IMP_HDF5_CALL(H5Pset_chunk(get_handle(), D, cdims));
  }
  HDF5DataSetCreationPropertiesD():
      HDF5DataSetAccessPropertiesD<TypeTraits, D>(H5P_DATASET_CREATE) {
    hsize_t cdims[D];
    cdims[0]=512;
    if (D >2) {
      std::fill(cdims+1, cdims+D-1, 4);
    }
    if (D >1) {
      cdims[D-1]=1;
    }
    IMP_HDF5_CALL(H5Pset_chunk(get_handle(), D, cdims));
    IMP_HDF5_CALL(H5Pset_fill_value(get_handle(),
                                    TypeTraits::get_hdf5_fill_type(),
                                      &TypeTraits::get_fill_value()));
    IMP_HDF5_CALL(H5Pset_fill_time(get_handle(), H5D_FILL_TIME_ALLOC));
    IMP_HDF5_CALL(H5Pset_alloc_time(get_handle(), H5D_ALLOC_TIME_INCR));

  }
};

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5DATA_SET_CREATION_PROPERTIES_D_H */
