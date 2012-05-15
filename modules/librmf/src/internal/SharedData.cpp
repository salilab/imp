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
namespace RMF {
  namespace internal {



    SharedData::~SharedData() {
      valid_=-66666;
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

  } // namespace internal
} /* namespace RMF */
