/**
 *  \file internal/reader_writer_utility.h
 *  \brief Utilities for reading/writing files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_READER_WRITER_UTILITY_H
#define IMPMULTIFIT_READER_WRITER_UTILITY_H

#include <IMP/multifit/multifit_config.h>
#include <string>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE
// work around clang bug
struct EmptyString {
  bool operator()(std::string s) const { return s.empty(); }
};
IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPMULTIFIT_READER_WRITER_UTILITY_H */
