/**
 *  \file IMP/base/Showable.h    \brief IO support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_SHOWABLE_H
#define IMPBASE_SHOWABLE_H

#include "base_config.h"
#include <IMP/compatibility/vector.h>
#include <vector>
#include <sstream>
#include <iostream>

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace std {
template <class T, class A>
class vector;
template <class A, class B>
class pair;
}
namespace IMP {
namespace compatibility {
template <class T>
class vector;
}
}
#include <sstream>
#endif

IMPBASE_BEGIN_NAMESPACE

class Object;
/** This is a helper class to aid in output of the various classes in \imp.
    To support output to streams, a class can use the IMP_SHOWABLE macros
    to define an implicit conversion to Showable.
*/
class IMPBASEEXPORT Showable {
  std::string str_;
  template <class T>
  void show_vector(const T &v) {
    std::ostringstream out;
    out << "[";
    for (unsigned int i=0; i< v.size(); ++i) {
      if (i >0) out << ", ";
      out << Showable(v[i]).str_;
    }
    out<< "]";
    str_= out.str();
  }
  template <class T>
      void show_ptr(const T*o) {
    std::ostringstream oss;
    if (o) {
      oss << '"' << o->get_name() << '"';
    } else {
      oss << "nullptr";
    }
    str_=oss.str();
  }
public:
  template <class T>
  explicit Showable(const T &t) {
    std::ostringstream oss;
    oss << t;
    str_= oss.str();
  }
  template <class T>
      explicit Showable(T *o) {
    show_ptr(o);
  }
  Showable(const std::string& str): str_(str){}
  template <class T, class TT>
  Showable(const std::pair<T, TT> &p) {
    std::ostringstream oss;
    oss << "(" << p.first << ", " << p.second << ")";
    str_=oss.str();
  }
  template <class T, class A>
  Showable(const std::vector<T, A> &v) {
    show_vector(v);
  }
  template <class T>
  Showable(const IMP::compatibility::vector<T> &v) {
    show_vector(v);
  }
  std::string get_string() const {
    return str_;
  }
  ~Showable();
};

inline std::ostream &operator<<(std::ostream &out, const Showable &s) {
  out << s.get_string();
  return out;
}


IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_SHOWABLE_H */
