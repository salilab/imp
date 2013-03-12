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
#include <RMF/HDF5/File.h>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/version.hpp>
#include <backend/hdf5/create.h>
#include <backend/avro/create.h>
#include <RMF/log.h>

RMF_ENABLE_WARNINGS

namespace RMF {
  namespace internal {

    SharedData::SharedData(std::string path): valid_(11111),
                                              cur_frame_(ALL_FRAMES),
                                              path_(path) {
      RMF_INFO(get_logger(), "Opening file " << path_);
    };
    SharedData::~SharedData() {
      RMF_INTERNAL_CHECK(valid_ == 11111,
                         "Already destroyed");
      valid_ = -66666;
      RMF_INFO(get_logger(), "Closing file " << path_);
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

    void SharedData::set_current_frame(int frame) {
    RMF_TRACE(get_logger(), "Setting current frame to " << frame);
    cur_frame_ = frame;
  }


    namespace {
      SharedData *create_shared_data_internal(std::string path, bool create,
                                              bool read_only) {
        try {
          SharedData *ret=NULL;
          if ((ret= hdf5_backend::create_shared_data(path, create, read_only))) {
            return ret;
          } else if ((ret= avro_backend::create_shared_data(path, create, read_only))) {
            return ret;
          } else {
            RMF_THROW(Message("Don't know how to open file"), IOException);
          }
        } catch (Exception &e) {
          RMF_RETHROW(File(path), e);
        }
      }
    }

    // throws RMF::IOException if couldn't create file or unsupported file
    // format
    SharedData* create_shared_data(std::string path, bool create) {
      return create_shared_data_internal(path, create, false);
    }

    SharedData* create_read_only_shared_data(std::string path) {
      return create_shared_data_internal(path, false, true);
    }

    SharedData* create_shared_data_in_buffer(std::string& buffer, bool create) {
      try {
        SharedData *ret=NULL;
        if ((ret= hdf5_backend::create_shared_data_buffer(buffer, create))) {
          return ret;
        } else if ((ret= avro_backend::create_shared_data_buffer(buffer, create))) {
          return ret;
        } else {
          RMF_THROW(Message("Don't know how to open file"), IOException);
        }
      } catch (Exception &e) {
        RMF_RETHROW(File("buffer"), e);
      }
    }

    SharedData* create_read_only_shared_data_from_buffer(const std::string &buffer) {
      try {
        SharedData *ret=NULL;
        if ((ret= hdf5_backend::create_shared_data_buffer(buffer))) {
          return ret;
        } else if ((ret= avro_backend::create_shared_data_buffer(buffer))) {
          return ret;
        } else {
          RMF_THROW(Message("Don't know how to open file"), IOException);
          }
      } catch (Exception &e) {
        RMF_RETHROW(File("buffer"), e);
      }
    }

    }   // namespace internal
  } /* namespace RMF */

RMF_DISABLE_WARNINGS
