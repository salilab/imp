/**
 *  \file IMP/base/WarningContext.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_CREATE_WARNING_CONTEXT_H
#define IMPKERNEL_CREATE_WARNING_CONTEXT_H

#include <IMP/base_config.h>
#include "showable_macros.h"
#include <boost/unordered_set.hpp>

IMPKERNEL_BEGIN_NAMESPACE
#if IMP_HAS_LOG
/** Warnings with the same key within the context are only output once.*/
struct IMPKERNELEXPORT WarningContext {
  mutable boost::unordered_set<std::string> data_;

 public:
  void add_warning(std::string key, std::string warning) const;
  void clear_warnings() const;
  void dump_warnings() const;
  ~WarningContext();
  IMP_SHOWABLE_INLINE(WarningContext, out << data_.size() << " warnings");
};
#else
struct IMPKERNELEXPORT WarningContext {
 public:
  WarningContext();
  void add_warning(std::string, std::string) const {}
  void clear_warnings() const {}
  void dump_warnings() const {}
  void show(std::ostream &) const {}
};
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CREATE_WARNING_CONTEXT_H */
