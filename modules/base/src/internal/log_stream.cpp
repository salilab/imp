/**
 *  \file static.cpp   \brief all static data for module.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/internal/log_stream.h"
#include <IMP/base/internal/static.h>
IMPBASE_BEGIN_INTERNAL_NAMESPACE
#if !IMP_BASE_HAS_LOG4CXX
struct IndentFilter: public boost::iostreams::output_filter {
  bool to_indent_;
  IndentFilter(): to_indent_(false){};
  template <typename Sink>
  bool put(Sink &sink, char c) {
    if (c=='\n') {
      to_indent_=true;
    } else if (to_indent_) {
      for (unsigned int i=0; i< log_indent; ++i) {
        boost::iostreams::put(sink, ' ');
      }
      if (print_time) {
        std::ostringstream oss;
          oss << log_timer.elapsed();
          std::string str= oss.str();
          for (unsigned int i=0; i< str.size(); ++i) {
            boost::iostreams::put(sink, str[i]);
          }
          boost::iostreams::put(sink, ':');
          boost::iostreams::put(sink, ' ');
      }
      to_indent_=false;
    }
    return boost::iostreams::put(sink, c);
  }
};

struct LogSink: boost::iostreams::sink {
  LogStream *ps_;
  LogSink(LogStream *ps): ps_(ps){}
  unsigned int write(const char *s, std::streamsize n) {
    ps_->get_stream().get_stream().write(s, n);
    return n;
  }
};

LogStream::LogStream(): out_(TextOutput(std::cout)) {
    P::push(IndentFilter());
    P::push(LogSink(this));
  }
void LogStream::set_stream(TextOutput out) {
  // temporarily disable writes, otherwise at log level MEMORY the log is
  // displayed using the old out_ object, which is in the process of being
  // freed (generally this leads to a segfault)
  LogLevel old = get_log_level();
  set_log_level(SILENT);
  out_=out;
  set_log_level(old);
}

LogStream::~LogStream() {
  // make sure nothing is written during destruction
  set_log_level(SILENT);
}
#endif // IMP_BASE_HAS_LOG4CXX

IMPBASE_END_INTERNAL_NAMESPACE
