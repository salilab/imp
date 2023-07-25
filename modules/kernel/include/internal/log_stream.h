/**
 *  \file internal/log_stream.h
 *  \brief Various useful utilities for logging
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LOG_STREAM_H
#define IMPKERNEL_INTERNAL_LOG_STREAM_H

#include <IMP/kernel_config.h>
#if !IMP_KERNEL_HAS_LOG4CXX

#include "../file.h"
#include "base_static.h"
#include "SimpleTimer.h"
#include <istream>
#include <sstream>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>

IMPKERNEL_BEGIN_NAMESPACE
namespace internal {

IMPKERNELEXPORT extern SimpleTimer log_timer;

class LogStream
    : public boost::iostreams::filtering_stream<boost::iostreams::output>,
      public boost::noncopyable {
  typedef boost::iostreams::filtering_stream<boost::iostreams::output> P;
  TextOutput out_;
  std::string prefix_;

 public:
  LogStream();
  virtual ~LogStream();
  void set_stream(TextOutput out);
  TextOutput get_stream() const { return out_; }
};
}
IMPKERNEL_END_NAMESPACE
#endif  // IMP_KERNEL_HAS_LOG4CXX
#endif  /* IMPKERNEL_INTERNAL_LOG_STREAM_H */
