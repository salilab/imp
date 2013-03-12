/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_HDF5_DATA_SET_CACHE_D_H
#define RMF_INTERNAL_HDF5_DATA_SET_CACHE_D_H

#include <RMF/config.h>
#include <RMF/HDF5/DataSetD.h>
#include <boost/utility.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace hdf5_backend {

template <class Traits, unsigned int D>
class HDF5DataSetCacheD {
};

}   // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_HDF5_DATA_SET_CACHE_D_H */
