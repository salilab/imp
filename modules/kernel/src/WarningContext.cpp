/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/WarningContext.h"
#include "IMP/log_macros.h"

IMPKERNEL_BEGIN_NAMESPACE
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
void WarningContext::clear_warnings() const { data_.clear(); }
void WarningContext::dump_warnings() const {}

WarningContext::~WarningContext() {}
#else
WarningContext::WarningContext() {}
#endif
IMPKERNEL_END_NAMESPACE
