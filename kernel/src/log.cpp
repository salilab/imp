/**
 *  \file Log.cpp   \brief Logging and error reporting support.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/internal/log_internal.h"
#include "IMP/exception.h"
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE

LogLevel log_level= TERSE;
LogTarget log_target= COUT;
unsigned int log_indent=0;

namespace {
  double initialized=11111111;
  std::ofstream fstream;
  boost::iostreams::filtering_stream<boost::iostreams::output> filtering;

  struct IndentFilter: public boost::iostreams::output_filter {
    bool to_indent;
    IndentFilter(): to_indent(false){};
    template <typename Sink>
    bool put(Sink &sink, char c) {
      if (c=='\n') {
        to_indent=true;
      } else if (to_indent) {
        for (unsigned int i=0; i< log_indent; ++i) {
          boost::iostreams::put(sink, ' ');
        }
        to_indent=false;
      }
      return boost::iostreams::put(sink, c);
    }
  };

  struct LogSink: boost::iostreams::sink {
    unsigned int write(const char *s, std::streamsize n) {
      if (log_target== COUT) {
        std::cout.write(s, n);
      } else if (log_target== CERR) {
        std::cerr.write(s, n);
      } else {
        fstream.write(s, n);
      }
      return n;
    }
  };

  struct SetupStream {
    SetupStream() {
      filtering.push(IndentFilter());
      filtering.push(LogSink());
    }
  } stream_setup_helper;
}


IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE


void set_log_target(LogTarget l)
{
  internal::log_target=l;
}

void set_log_file(std::string l) {
  if (!l.empty()) {
    internal::fstream.open(l.c_str());
    if (!internal::fstream.is_open()) {
      IMP_failure("Error opening log file " << l, ValueException);
    } else {
      internal::log_target=FILE;
    }
  } else {
    internal::fstream.close();
    internal::log_target=COUT;
  }
}

void log_write(std::string str) {
  IMP_assert(internal::initialized=11111111,
             "You connot use the log before main is called.");
  internal::filtering.write(str.c_str(), str.size());
  internal::filtering.strict_sync();
}

IMP_END_NAMESPACE
