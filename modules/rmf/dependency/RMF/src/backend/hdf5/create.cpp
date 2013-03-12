/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "create.h"
#include "HDF5SharedData.h"
#include <boost/algorithm/string/predicate.hpp>
#include <RMF/internal/map.h>
#include <RMF/log.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace hdf5_backend {

  internal::SharedData* create_shared_data(std::string path, bool create,
                                           bool read_only) {
   if (!boost::algorithm::ends_with(path, ".rmf")) {
      return NULL;
    }
    RMF_INFO(get_logger(), "Using HDF5 hdf5_backend");
    HDF5SharedData*ret= new HDF5SharedData(path, create, read_only);
    return ret;
  }
  internal::SharedData* create_shared_data_buffer(std::string &/*buffer*/,
                                                  bool /*create*/) {
    return NULL;
  }
  internal::SharedData* create_shared_data_buffer(const std::string &/*buffer*/) {
    return NULL;
  }

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
