/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/SharedData.h>
#include <RMF/NodeHandle.h>
#include <RMF/NodeSetHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
#include <RMF/HDF5File.h>
#include <RMF/internal/HDF5SharedData.h>
#include <RMF/internal/ProtoBufSharedData.h>
#include <boost/algorithm/string/predicate.hpp>
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
          for (map<std::string, SharedData *>::const_iterator it=cache.begin();
               it != cache.end(); ++it) {
            std::cerr << it->first << std::endl;
          }
        }
      }
    };
    CacheCheck checker;
    }
    SharedData::SharedData(): valid_(11111) {
    };
    SharedData::~SharedData() {
      valid_=-66666;
      // check for an exception in the constructor
      if (reverse_cache.find(this) != reverse_cache.end()) {
        std::string name= reverse_cache.find(this)->second;
        cache.erase(name);
        reverse_cache.erase(this);
      }
    }

    void SharedData::audit_key_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\\:=()[]{}\"'";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW(get_error_message("Key names can't contain ",
                                          *cur), UsageException);
        }
        ++cur;
      }
      if (name.find("  ") != std::string::npos) {
        IMP_RMF_THROW("Key names can't contain two consecutive spaces",
                      UsageException);
      }
    }

    void SharedData::audit_node_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\"";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW(get_error_message("Node names names can't contain \"",
                                          *cur,
                                          "\", but \"", name, "\" does."),
                        UsageException);
        }
        ++cur;
      }
    }


  void SharedData::validate() const {
    Creators cs= get_validators();
    for (unsigned int i=0; i< cs.size(); ++i) {
      boost::scoped_ptr<Validator>
          ptr(cs[i]->create(FileHandle(const_cast<SharedData*>(this))));
      ptr->write_errors(std::cerr);
    }
  }


    SharedData* create_shared_data(std::string path, bool create) {
      SharedData *ret;
      if (cache.find(path) != cache.end()) {
        return cache.find(path)->second;
      }
      if (boost::algorithm::ends_with(path, ".rmf")) {
        HDF5Group g;
        if (create) {
          g= create_hdf5_file(path);
        } else {
          g= open_hdf5_file(path);
        }
        ret= new HDF5SharedData(g, create);
#ifdef RMF_USE_PROTOBUF
      } else if (boost::algorithm::ends_with(path, ".prmf")) {
        ret= new ProtoBufSharedData(path, create);
#endif
      } else {
        IMP_RMF_THROW("Don't know how to open file", IOException);
      }
      cache[path]=ret;
      reverse_cache[ret]=path;
      return ret;
    }

    SharedData* create_read_only_shared_data(std::string path) {
      SharedData *ret;
      if (cache.find(path) != cache.end()) {
        return cache.find(path)->second;
      }
      if (boost::algorithm::ends_with(path, ".rmf")) {
        HDF5ConstGroup g;
        g = open_hdf5_file_read_only(path);
        ret= new HDF5SharedData(HDF5Group::get_from_const_group(g), false);
#ifdef RMF_USE_PROTOBUF
      } else if (boost::algorithm::ends_with(path, ".prmf")) {
        ret= new ProtoBufSharedData(path, false);
#endif
      } else {
        IMP_RMF_THROW("Don't know how to open file", IOException);
      }
      cache[path]=ret;
      reverse_cache[ret]=path;
      return ret;
    }

  } // namespace internal
} /* namespace RMF */
