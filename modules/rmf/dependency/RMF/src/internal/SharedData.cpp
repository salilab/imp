/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/SharedData.h>
#include <RMF/NodeHandle.h>
#include <RMF/internal/set.h>
#include <RMF/HDF5File.h>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/version.hpp>
#include "../backend/hdf5/HDF5SharedData.h"
#include "../backend/avro/AvroSharedData.h"
#include "../backend/avro/SingleAvroFile.h"
#include "../backend/avro/MultipleAvroFileWriter.h"
#include "../backend/avro/MultipleAvroFileReader.h"

namespace RMF {
namespace internal {


namespace {
map<std::string, SharedData *> cache;
map<SharedData*, std::string> reverse_cache;

struct CacheCheck {
  ~CacheCheck() {
    if (!cache.empty()) {
      std::cerr << "Not all open objects were properly close before the rmf"
                << " module was unloaded. This is a bad thing."
                << std::endl;
      for (map<std::string, SharedData *>::const_iterator it = cache.begin();
           it != cache.end(); ++it) {
        std::cerr << it->first << std::endl;
      }
    }
  }
};
CacheCheck checker;
}
SharedData::SharedData(std::string path): valid_(11111),
                                          cur_frame_(ALL_FRAMES),
                                          path_(path) {
};
SharedData::~SharedData() {
  RMF_INTERNAL_CHECK(valid_ == 11111,
                     "Already destroyed");
  valid_ = -66666;
  // check for an exception in the constructor
  if (reverse_cache.find(this) != reverse_cache.end()) {
    std::string name = reverse_cache.find(this)->second;
    cache.erase(name);
    reverse_cache.erase(this);
  }
}

void SharedData::audit_key_name(std::string name) const {
  if (name.empty()) {
        RMF_THROW(Message("Empty key name"), UsageException);
  }
  static const char *illegal = "\\:=()[]{}\"'";
  const char *cur = illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
        RMF_THROW(Message(get_error_message("Key names can't contain ",
                                          *cur)), UsageException);
    }
    ++cur;
  }
  if (name.find("  ") != std::string::npos) {
        RMF_THROW(Message("Key names can't contain two consecutive spaces"),
              UsageException);
  }
}

void SharedData::audit_node_name(std::string name) const {
  if (name.empty()) {
        RMF_THROW(Message("Empty key name"), UsageException);
  }
  static const char *illegal = "\"";
  const char *cur = illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
        RMF_THROW(Message(get_error_message("Node names names can't contain \"",
                                          *cur,
                                          "\", but \"", name, "\" does.")),
                UsageException);
    }
    ++cur;
  }
}

std::string SharedData::get_file_name() const {
#if BOOST_VERSION >= 104600
  return boost::filesystem::path(path_).filename().string();
#else
  return boost::filesystem::path(path_).filename();
#endif
}


typedef AvroSharedData<SingleAvroFile> SingleAvroShareData;
typedef AvroSharedData<MultipleAvroFileWriter> AvroWriterShareData;
typedef AvroSharedData<MultipleAvroFileReader> AvroReaderShareData;

// throws RMF::IOException if couldn't create file or unsupported file
// format
SharedData* create_shared_data(std::string path, bool create) {
  SharedData *ret;
  try {
    if (boost::algorithm::ends_with(path, ".rmf")) {
      if (cache.find(path) != cache.end()) {
        return cache.find(path)->second;
      }
      ret = new HDF5SharedData(path, create, false);
      cache[path] = ret;
      reverse_cache[ret] = path;
    } else if (boost::algorithm::ends_with(path, ".rmfa")) {
      ret = new SingleAvroShareData(path, create, false);
    } else if (boost::algorithm::ends_with(path, ".rmf2")) {
      if (create) {
        ret = new AvroWriterShareData(path, create, false);
      } else {
        RMF_THROW(Message("RMF2 files can only be opened with " \
                          "open_rmf_file_read_only() or create_rmf_file()"),
                  IOException);
      }
    } else {
        RMF_THROW(Message("Don't know how to open file"), IOException);
    }
  } catch (Exception &e) {
    RMF_RETHROW(File(path), e);
  }
  return ret;
}

SharedData* create_read_only_shared_data(std::string path) {
  SharedData *ret;
  try {
    if (boost::algorithm::ends_with(path, ".rmf")) {
      if (cache.find(path) != cache.end()) {
        return cache.find(path)->second;
      }
      ret = new HDF5SharedData(path, false, true);
      cache[path] = ret;
      reverse_cache[ret] = path;
    } else if (boost::algorithm::ends_with(path, ".rmfa")) {
      ret = new SingleAvroShareData(path, false, true);
    } else if (boost::algorithm::ends_with(path, ".rmf2")) {
      ret = new AvroReaderShareData(path, false, true);
    } else {
      RMF_THROW(Message("Don't know how to open file"), IOException);
    }
  } catch (Exception &e) {
    RMF_RETHROW(File(path), e);
  }
  return ret;
}

SharedData* create_shared_data_in_buffer(std::string& buffer, bool create) {
  return new SingleAvroShareData(buffer, create, false, true);
}

SharedData* create_read_only_shared_data_from_buffer( std::string buffer) {
  return new SingleAvroShareData(buffer, false, true, true);
}

}   // namespace internal
} /* namespace RMF */
