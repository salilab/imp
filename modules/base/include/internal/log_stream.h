/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_LOG_STREAM_H
#define IMPBASE_INTERNAL_LOG_STREAM_H

#include <IMP/base/base_config.h>
#if !IMP_BASE_HAS_LOG4CXX

#include "../file.h"
#include "static.h"
#include <istream>
#include <sstream>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/timer.hpp>


IMPBASE_BEGIN_NAMESPACE
namespace internal {

IMPBASEEXPORT extern boost::timer log_timer;

class LogStream:
  public boost::iostreams::filtering_stream<boost::iostreams::output>,
  public boost::noncopyable  {
  typedef boost::iostreams::filtering_stream<boost::iostreams::output> P;
  TextOutput out_;
  std::string prefix_;
 public:
  LogStream();
  virtual ~LogStream();
  void set_stream(TextOutput out);
  TextOutput get_stream() const {
    return out_;
  }
};
}
IMPBASE_END_NAMESPACE
#endif // IMP_BASE_HAS_LOG4CXX
#endif  /* IMPBASE_INTERNAL_LOG_STREAM_H */
