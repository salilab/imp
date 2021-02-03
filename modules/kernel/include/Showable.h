/**
 *  \file IMP/Showable.h    \brief IO support.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SHOWABLE_H
#define IMPKERNEL_SHOWABLE_H

#include <IMP/kernel_config.h>
// do not include anything else from IMP
#include <sstream>
#include <iostream>
#include <utility>
#include <sstream>

IMPKERNEL_BEGIN_NAMESPACE
/** This is a helper class to aid in output of the
       various classes in \imp.
       To support output to streams, a class can use the
       IMP_SHOWABLE macros
       to define an implicit conversion to Showable.
   */
class IMPKERNELEXPORT Showable {
  std::string str_;
  template <class T>
  void show_ptr(const T *o) {
    std::ostringstream oss;
    if (o) {
      oss << '"' << o->get_name() << '"';
    } else {
      oss << "nullptr";
    }
    str_ = oss.str();
  }

 public:
  template <class T>
  explicit Showable(const T &t) {
    std::ostringstream oss;
    oss << t;
    str_ = oss.str();
  }
  template <class T>
  explicit Showable(const T *o) {
    show_ptr(o);
  }
  template <class T>
  explicit Showable(T *o) {
    show_ptr(o);
  }
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(Showable);
  Showable(const std::string &str) : str_(str) {}
  Showable(const char *str) : str_(str) {}
  template <class T, class TT>
  Showable(const std::pair<T, TT> &p) {
    std::ostringstream oss;
    oss << "(" << p.first << ", " << p.second << ")";
    str_ = oss.str();
  }
  std::string get_string() const { return str_; }
  ~Showable();
};

inline std::ostream &operator<<(std::ostream &out, const Showable &s) {
  out << s.get_string();
  return out;
}

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SHOWABLE_H */
