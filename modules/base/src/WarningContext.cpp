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
#if IMP_HAS_LOG >= IMP_WARN
    if (data_.find(key) == data_.end()) {
      data_.insert(key);
      IMP_WARN(warning);
    }
#endif
  }
void WarningContext::clear_warnings() const {
    data_.clear();
  }
void WarningContext::dump_warnings() const {
}

WarningContext::~WarningContext() {
}
#else
WarningContext::WarningContext(){}
#endif
IMPBASE_END_NAMESPACE
