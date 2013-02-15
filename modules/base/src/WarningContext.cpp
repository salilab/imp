/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/WarningContext.h"
#include "IMP/base/log_macros.h"

IMPBASE_BEGIN_NAMESPACE
#if IMP_HAS_LOG
void WarningContext::add_warning(std::string key, std::string warning) const {
    if (warning.empty()) return;
    IMP_IF_LOG(WARNING) {
      if (data_.find(key) == data_.end()) {
        data_[key]=warning;
      }
    }
  }
void WarningContext::clear_warnings() const {
    data_.clear();
  }
void WarningContext::dump_warnings() const {
  for (base::map<std::string, std::string>::iterator it= data_.begin();
       it != data_.end(); ++it) {
    if (!it->second.empty()) {
      IMP_WARN(it->second << std::endl);
      it->second=std::string();
    }
  }
}

WarningContext::~WarningContext() {
  dump_warnings();
}
#else
WarningContext::WarningContext(){}
#endif
IMPBASE_END_NAMESPACE
