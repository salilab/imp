/**
 *  \file PrefixStream.h
 *  \brief A stream which prefixes each line with a string.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_PREFIX_STREAM_H
#define IMPKERNEL_INTERNAL_PREFIX_STREAM_H

#include <IMP/kernel/kernel_config.h>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

class PrefixStream:
  public boost::iostreams::filtering_stream<boost::iostreams::output>,
  public boost::noncopyable  {
  typedef boost::iostreams::filtering_stream<boost::iostreams::output> P;
  std::ostream *out_;
  std::string prefix_;

  struct IndentFilter: public boost::iostreams::output_filter {
    PrefixStream *ps_;
    bool to_indent_;
    IndentFilter(PrefixStream *ps): ps_(ps), to_indent_(false){};
    template <typename Sink>
    bool put(Sink &sink, char c) {
      if (c=='\n') {
        to_indent_=true;
      } else if (to_indent_) {
        for (unsigned int i=0; i< ps_->prefix_.size(); ++i) {
          boost::iostreams::put(sink, ps_->prefix_[i]);
        }
        to_indent_=false;
      }
      return boost::iostreams::put(sink, c);
    }
  };

  struct LogSink: boost::iostreams::sink {
    PrefixStream *ps_;
    LogSink(PrefixStream *ps): ps_(ps){}
    unsigned int write(const char *s, std::streamsize n) {
      ps_->out_->write(s, n);
      return n;
    }
  };
  friend struct LogSink;
  friend struct IndentFilter;
 public:
  PrefixStream(std::ostream *out): out_(out) {
    P::push(IndentFilter(this));
    P::push(LogSink(this));
  }
  std::string get_prefix() const {
    return prefix_;
  }
  void set_prefix(std::string prefix) {
    prefix_=prefix;
  }
  void set_stream(std::ostream *out) {
    out_=out;
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_PREFIX_STREAM_H */
