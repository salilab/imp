/**
 *  \file RMF/HDF5/DataSetAccessPropertiesD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_DATA_SET_ACCESS_PROPERTIES_D_H
#define RMF_HDF5_DATA_SET_ACCESS_PROPERTIES_D_H

#include "RMF/config.h"
#include "DataSetIndexD.h"
#include <memory>

RMF_ENABLE_WARNINGS namespace RMF {
  namespace HDF5 {

  /** A class to manage properties controlling access to HDF5 data sets.*/
  template <class TypeTraits, unsigned int D>
  class DataSetAccessPropertiesD {
    std::shared_ptr<SharedHandle> h_;

   protected:
    DataSetAccessPropertiesD(hid_t type)
        : h_(new SharedHandle(H5Pcreate(type), &H5Pclose, "Properties")) {}

   public:
    hid_t get_handle() const { return *h_; }
    /* Set the chunk cache to a certain size. See
       \external{https://docs.hdfgroup.org/releases/hdf5/v1_14/v1_14_4/documentation/doxygen/group___d_a_p_l.html#ga104d00442c31714ee073dee518f661f1,H5Pset_chunk_cache}.
     */
    void set_chunk_cache_size(unsigned int entries, unsigned int size_bytes,
                              double policy = .75) {
      RMF_HDF5_CALL(
          H5Pset_chunk_cache(get_handle(), entries, size_bytes, policy));
    }
    DataSetAccessPropertiesD()
        : h_(new SharedHandle(H5Pcreate(H5P_DATASET_ACCESS), &H5Pclose,
                              "Properties")) {}
  };

  } /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5_DATA_SET_ACCESS_PROPERTIES_D_H */
