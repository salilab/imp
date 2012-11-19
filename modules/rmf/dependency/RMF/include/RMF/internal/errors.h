/**
 *  \file RMF/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__INTERNAL_ERRORS_H
#define RMF__INTERNAL_ERRORS_H

#include <RMF/config.h>
#include "../infrastructure_macros.h"
#include <sstream>

namespace RMF {
namespace internal {

#ifdef __GNUC__
  template <class E>
  __attribute__((noreturn)) void handle_error(std::string message);
#endif
  template <class E>
  void handle_error(std::string message) {
    throw E(message.c_str());
  }

extern RMFEXPORT bool show_hdf5_errors;

  template <class A>
  std::string get_error_message(const A &a) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a;
    return a.str();
  }

  template <class A, class B>
  std::string get_error_message(const A &a,
                                const B &b) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b;
    return oss.str();
  }

  template <class A, class B, class C>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c;
    return oss.str();
  }

  template <class A, class B, class C, class D>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d;
    return oss.str();
  }

  template <class A, class B, class C, class D, class E>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e;
    return oss.str();
  }

  template <class A, class B, class C, class D, class E, class F>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f;
    return oss.str();
  }

  template <class A, class B, class C, class D, class E, class F, class G>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f,
                                const G &g) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f << g;
    return oss.str();
  }

  template <class A, class B, class C, class D, class E, class F,
            class G, class H>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f,
                                const G &g,
                                const H &h) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f << g << h;
    return oss.str();
  }
  template <class A, class B, class C, class D, class E, class F,
            class G, class H, class I>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f,
                                const G &g,
                                const H &h,
                                const I &i) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f << g << h << i;
    return oss.str();
  }


  template <class A, class B, class C, class D, class E, class F,
            class G, class H, class I, class J>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f,
                                const G &g,
                                const H &h,
                                const I &i,
                                const J &j) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f << g << h << i << j;
    return oss.str();
  }

  template <class A, class B, class C, class D, class E, class F,
            class G, class H, class I, class J, class K>
  std::string get_error_message(const A &a,
                                const B &b,
                                const C &c,
                                const D &d,
                                const E &e,
                                const F &f,
                                const G &g,
                                const H &h,
                                const I &i,
                                const J &j,
                                const K &k) {
    using RMF::operator<<;
    std::ostringstream oss;
    oss << a << b << c << d << e << f << g << h << i << j << k;
    return oss.str();
  }

} // namespace internal
} /* namespace RMF */

#endif /* RMF__INTERNAL_ERRORS_H */
