/**
 *  \file base/internal/OwnerPointer.h
 *  \brief A nullptr-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_INTERNAL_POINTER_BASE_H
#define IMPBASE_INTERNAL_POINTER_BASE_H

#include "ref_counting.h"
#include "../check_macros.h"
#include "../warning_macros.h"
#include "IMP/base/hash.h"
#include "IMP/base/nullptr.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>

IMPBASE_BEGIN_INTERNAL_NAMESPACE

template <class TT>
struct RefCountedPointerTraits {
  typedef TT Type;
  static void handle_set(TT* t) {
    IMP_CHECK_OBJECT_IF_NOT_nullptr(t);
    internal::ref(t);
  }
  static void handle_unset(TT* t) {
    internal::unref(t);
  }
  static void check(const TT *o) {
    IMP_CHECK_OBJECT(o);
  }
};
template <class TT>
struct OwnerPointerTraits: public RefCountedPointerTraits<TT> {
  typedef TT Type;
  static void handle_set(TT* t) {
    t->set_was_used(true);
    RefCountedPointerTraits<TT>::handle_set(t);
  }
};
template <class TT>
struct WeakPointerTraits {
  typedef TT Type;
  static void handle_set(TT* ) {
  }
  static void handle_unset(TT* ) {
  }
  static void check(const TT *) {
    // needs to support incomplete types
    //IMP_CHECK_OBJECT(o);
  }
};

template <class TT>
struct CheckedWeakPointerTraits {
  typedef TT Type;
  static void handle_set(TT*o) {
    IMP_CHECK_VARIABLE(o);
    IMP_CHECK_OBJECT_IF_NOT_nullptr(o);
  }
  static void handle_unset(TT* ) {
  }
  static void check(const TT *o) {
    IMP_CHECK_OBJECT(o);
  }
};


template <class O, class OO, class Enabled=void>
  struct GetPointer {
  static O* get_pointer(const OO& o) {
    return o;
  }
  static const O* get_const_pointer(const OO& o) {
    return o;
  }
};
#if !IMP_COMPILER_HAS_NULLPTR
template <class O, class OO>
struct GetPointer<O, OO,
                    typename boost::enable_if<boost::mpl::and_<
                         boost::mpl::not_<boost::is_integral<OO> >,
             boost::mpl::not_<boost::is_pointer<OO> > > >::type> {
    static O* get_pointer(const OO& o) {
      return o;
    }
    static const O* get_const_pointer(const OO& o) {
      return o;
    }
  };
template <class O, class OO>
struct GetPointer<O, OO*,
                  typename boost::enable_if<boost::is_pointer<OO*>
                                            >::type> {
    static O* get_pointer(OO* o) {
      return o;
    }
    static const O* get_const_pointer(const OO* o) {
      return o;
    }
  };


template <class O, class OO>
struct GetPointer<O, OO,
                    typename boost::enable_if<boost::is_integral<OO>
                                              >::type> {
  static O* get_pointer(const OO& o) {
    IMP_INTERNAL_CHECK_VARIABLE(o)
    IMP_INTERNAL_CHECK(o==0, "Non-zero pointer constant found.");
    return static_cast<O*>(nullptr);
  }
  static const O* get_const_pointer(const OO& o) {
    IMP_INTERNAL_CHECK_VARIABLE(o);
    IMP_INTERNAL_CHECK(o==0, "Non-zero pointer constant found.");
    return static_cast<O*>(nullptr);
  }
};
template <class O>
struct GetPointer<O, nullptr_t> {
  static O* get_pointer(const nullptr_t& ) {
    return static_cast<O*>(nullptr);
  }
  static const O* get_const_pointer(const nullptr_t& ) {
    return static_cast<O*>(nullptr);
  }
};

#endif

template <class Traits>
class PointerBase
{
public:
  typedef typename Traits::Type O;
private:
  O* o_;
  static void check(const O * o) {
    if (o) {
      Traits::check(o);
    }
  }
  static void check_non_null(const O*  t) {
    IMP_INTERNAL_CHECK(t, "Pointer is nullptr");
    check(t);
  }
  //static O* get_pointer(O*o) {return o;}
  /*static O* get_pointer(size_t t) {
    IMP_INTERNAL_CHECK(t==0, "Only can compare with nullptr ints");
    return nullptr;
    }*/


  template <class OO>
  static O* get_pointer(const OO& o) {
    return GetPointer<O, OO>::get_pointer(o);
  }
  template <class OO>
  static const O* get_const_pointer(const OO& o) {
    return GetPointer<O, OO>::get_const_pointer(o);
  }

  void set_pointer(O* p) {
    if (p) Traits::handle_set(p);
    if (o_) Traits::handle_unset(o_);
    o_=p;
  }

  struct UnusedClass{};
public:
  //! initialize to nullptr
  PointerBase(): o_(nullptr) {}
  /** drop control of the object */
  ~PointerBase(){
    if (o_) Traits::handle_unset(o_);
  }
  //! Return true if the pointer is not nullptr
  bool operator!() const {
    return !o_;
  }
  //! convert to the raw pointer
  operator O*() const {
    return o_;
  }
  IMP_HASHABLE_INLINE(PointerBase, return boost::hash_value(o_););
  const O& operator*() const {
    check_non_null(o_);
    return *o_;
  }
  O& operator*()  {
    check_non_null(o_);
    return *o_;
  }
  O* operator->() const {
    check_non_null(o_);
    return o_;
  }
  O* operator->() {
    check_non_null(o_);
    return o_;
  }
  //! get the raw pointer
  O* get() const {
    check_non_null(o_);
    return o_;
  }
  template <class OO>
  bool operator==(const OO& o) const {
    return (o_== get_const_pointer(o));
  }
  template <class OO>
  bool operator!=(const OO& o) const {
    return (o_!= get_const_pointer(o));
  }
  template <class OO>
  bool operator<(const OO& o) const {
    return (o_< get_const_pointer(o));
  }
  template <class OO>
  bool operator>(const OO& o) const {
    return (o_> get_const_pointer(o));
  }
  template <class OO>
  bool operator>=(const OO& o) const {
    return (o_>= get_const_pointer(o));
  }
  template <class OO>
  bool operator<=(const OO& o) const {
    return (o_<= get_const_pointer(o));
  }
  template <class OO>
  int compare(const OO& o) const {
    if (operator<(o)) return -1;
    else if (operator>(o)) return 1;
    else return 0;
  }
  template <class OO>
  explicit PointerBase(const OO& o): o_(nullptr) {
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    }
  }

  template <class OT>
  PointerBase<Traits>& operator=( const PointerBase<OT> &o){
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(nullptr);
    }
    return *this;
  }
  template <class OT>
    PointerBase<Traits>& operator=( OT* o){
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(nullptr);
    }
    return *this;
  }
#if !IMP_COMPILER_HAS_NULLPTR
  PointerBase<Traits>& operator=(nullptr_t) {
    set_pointer(nullptr);
    return *this;
  }
#endif
  PointerBase<Traits>& operator=(const PointerBase<Traits> &o) {
    set_pointer(o.o_);
    return *this;
  }
  /*IMP_POINTER_MEMBERS(template <class OO>,
    OO*);*/
  PointerBase(const PointerBase &o): o_(nullptr) {
    set_pointer(o.o_);
  }

  //! Relinquish control of the pointer
  /** This must be the only pointer pointing to the object. Its
      reference count will be 0 after the function is called, but
      the object will not be destroyed. Use this to safely return
      objects allocated within functions.
  */
  O* release() {
    internal::release(o_);
    O* ret=o_;
    o_= nullptr;
    return ret;
  }
  void swap_with(PointerBase<Traits> &o) {
    std::swap(o_, o.o_);
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class Traits>
inline void swap(PointerBase<Traits> &a, PointerBase<Traits> &b) {
  a.swap_with(b);
}

template <class OT, class OTraits>
inline bool operator==(OT *o, const PointerBase<OTraits> &p) {
  return p==o;
}
template <class OT, class OTraits>
inline bool operator!=(OT *o, const PointerBase<OTraits> &p) {
  return p!= o;
}
template <class OT, class OTraits>
inline bool operator<(OT *o, const PointerBase<OTraits> &p) {
  return p > o;
}
template <class OT, class OTraits>
inline bool operator>(OT *o, const PointerBase<OTraits> &p) {
  return p < o;
}
template <class OT, class OTraits>
inline bool operator>=(OT *o, const PointerBase<OTraits> &p) {
  return p <= o;
}
template <class OT, class OTraits>
inline bool operator<=(OT *o, const PointerBase<OTraits> &p) {
  return p >= o;
}
#endif

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_POINTER_BASE_H */
