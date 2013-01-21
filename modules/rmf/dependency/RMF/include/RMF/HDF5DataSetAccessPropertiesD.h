/**
 *  \file RMF/HDF5DataSetAccessPropertiesD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5DATA_SET_ACCESS_PROPERTIES_D_H
#define RMF_HDF_5DATA_SET_ACCESS_PROPERTIES_D_H

#include <RMF/config.h>
#include "types.h"
#include "HDF5DataSetIndexD.h"


namespace RMF {

/** A class to manage properties controlling access to HDF5 data sets.*/
template <class TypeTraits, unsigned int D>
class HDF5DataSetAccessPropertiesD {
  boost::intrusive_ptr<HDF5SharedHandle> h_;
protected:
  HDF5DataSetAccessPropertiesD(hid_t type):
    h_(new HDF5SharedHandle(H5Pcreate(type), &H5Pclose,
                            "Properties")) {
  }
public:
  hid_t get_handle() const {
    return *h_;
  }
  /* Set the chunk cache to a certain size. See
     \external{http://www.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetChunkCache,H5Pset_chunk_cache}.
   */
  void set_chunk_cache_size(unsigned int entries,
                            unsigned int size_bytes,
                            double       policy = .75) {
    RMF_HDF5_CALL(H5Pset_chunk_cache(get_handle(), entries,
                                     size_bytes, policy));
  }
  HDF5DataSetAccessPropertiesD():
    h_(new HDF5SharedHandle(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose,
                            "Properties")) {
  }
};


} /* namespace RMF */

#endif /* RMF_HDF_5DATA_SET_ACCESS_PROPERTIES_D_H */
