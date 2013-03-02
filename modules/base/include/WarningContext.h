/**
 *  \file IMP/base/WarningContext.h
 *  \brief Logging and error reporting support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_CREATE_WARNING_CONTEXT_H
#define IMPBASE_CREATE_WARNING_CONTEXT_H

#include <IMP/base/base_config.h>
#include "showable_macros.h"
#include <IMP/base/set.h>

IMPBASE_BEGIN_NAMESPACE
#if IMP_HAS_CHECKS
/** Warnings with the same key within the context are only output once.*/
struct IMPBASEEXPORT WarningContext {
  mutable base::set<std::string> data_;
public:
  void add_warning(std::string key, std::string warning) const;
  void clear_warnings() const;
  void dump_warnings() const;
  ~WarningContext();
  IMP_SHOWABLE_INLINE(WarningContext, out << data_.size() << " warnings");
};
#else
struct IMPBASEEXPORT WarningContext {
public:
  WarningContext();
  void add_warning(std::string, std::string ) const {}
  void clear_warnings() const {}
  void dump_warnings() const {}
  void show(std::ostream &) const {}
};
#endif


IMPBASE_END_NAMESPACE


#endif  /* IMPBASE_CREATE_WARNING_CONTEXT_H */
