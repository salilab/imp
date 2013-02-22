/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/compiler_warnings.h>
#include "create.h"
#include "HDF5SharedData.h"
#include <boost/algorithm/string/predicate.hpp>
#include <RMF/internal/map.h>
#include <RMF/log.h>

namespace RMF {
namespace hdf5_backend {

  internal::map<std::string, HDF5SharedData *> cache;
  internal::map<HDF5SharedData*, std::string> reverse_cache;

  struct CacheCheck {
    ~CacheCheck() {
      if (!cache.empty()) {
        std::cerr << "Not all open objects were properly close before the rmf"
                  << " module was unloaded. This is a bad thing."
                  << std::endl;
        for (internal::map<std::string, HDF5SharedData *>::const_iterator it = cache.begin();
             it != cache.end(); ++it) {
          std::cerr << it->first << std::endl;
        }
      }
    }
  };
  CacheCheck checker;


  internal::SharedData* create_shared_data(std::string path, bool create,
                                      bool read_only) {
   if (!boost::algorithm::ends_with(path, ".rmf")) {
      return NULL;
    }
    if (cache.find(path) != cache.end()) {
      return cache.find(path)->second;
    }
    RMF_INFO(get_logger(), "Using HDF5 hdf5_backend");
    HDF5SharedData*ret= new HDF5SharedData(path, create, read_only);
    cache[path]=ret;
    reverse_cache[ret]=path;
    return ret;
  }
  internal::SharedData* create_shared_data_buffer(std::string &buffer,
                                             bool create) {
    return NULL;
  }
  internal::SharedData* create_shared_data_buffer(const std::string &buffer) {
    return NULL;
  }

}   // namespace avro_backend
} /* namespace RMF */
