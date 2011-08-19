/**
 *  \file OwnerPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_POINTER_BASE_H
#define IMP_INTERNAL_POINTER_BASE_H


#include "../Object.h"
#include "ref_counting.h"
#include "IMP/compatibility/hash.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE

struct RefCountedPointerTraits {
  template <class T>
  static void handle_set(T* t) {
    internal::ref(t);
  }
  template <class T>
  static void handle_unset(T* t) {
    internal::unref(t);
  }
  static void check(const RefCounted *o) {
    IMP_INTERNAL_CHECK(o->get_ref_count() >0, "Ref count is null");
  }
  static void check(const Object *o) {
    IMP_CHECK_OBJECT(o);
  }
};

struct OwnerPointerTraits: public RefCountedPointerTraits {
  template <class T>
  static void handle_set(T* t) {
    t->set_was_used(true);
    RefCountedPointerTraits::handle_set(t);
  }
};
struct WeakPointerTraits {
  template <class T>
  static void handle_set(T* ) {
  }
  template <class T>
  static void handle_unset(T* ) {
  }
  template <class T>
  static void check(T*){}
  static void check(const RefCounted *o) {
    IMP_INTERNAL_CHECK(o->get_ref_count() >0, "Ref count is null");
  }
  static void check(const Object *o) {
    IMP_CHECK_OBJECT(o);
  }
};

template <class O, class Traits>
class PointerBase
{
  O* o_;
  static void check(const O *o) {
    if (o) {
      Traits::check(o);
    }
  }
  static void check_non_null(const O*t) {
    IMP_INTERNAL_CHECK(t != NULL, "Pointer is NULL");
    check(t);
  }
  static O* get_pointer(O*o) {return o;}
  static O* get_pointer(size_t t) {
    IMP_INTERNAL_CHECK(t==0, "Only can compare with NULL ints");
    return NULL;
  }
  static O* get_pointer(const PointerBase<O, Traits>&o) {return o.o_;}
  void set_pointer(O* p) {
    if (p == o_) return;
    if (o_) Traits::handle_unset(o_);
    if (p) Traits::handle_set(p);
    check(p);
    o_=p;
  }
public:
  //! This is needed as the template one is not reliably invoked
  PointerBase(const PointerBase &o): o_(NULL) {
    if (o) {
      set_pointer(o.get());
    }
  }
  template <class OT, class OTraits>
  explicit PointerBase(const PointerBase<OT, OTraits> &o): o_(NULL) {
    if (o) {
      set_pointer(o.get());
    }
  }
  //! initialize to NULL
  PointerBase(): o_(NULL) {}
  /** initialize from a pointer */
  explicit PointerBase(O* o): o_(NULL) {
    set_pointer(o);
  }
  /** initialize from a pointer */
  explicit PointerBase(const long int o): o_(NULL) {
    IMP_USAGE_CHECK(o==0, "Non-null constant used for pointer.");
  }
  /** drop control of the object */
  ~PointerBase(){
    set_pointer(NULL);
  }
  template <class OT>
  bool operator==(const OT &o) const {
    return (o_== get_pointer(o));
  }
  template <class OT>
  bool operator!=(const OT &o) const {
    return (o_!= get_pointer(o));
  }
  template <class OT>
  bool operator<(const OT &o) const {
    return (o_< get_pointer(o));
  }
  template <class OT>
  bool operator>(const OT &o) const {
    return (o_> get_pointer(o));
  }
  template <class OT>
  bool operator>=(const OT &o) const {
    return (o_>= get_pointer(o));
  }
  template <class OT>
  bool operator<=(const OT &o) const {
    return (o_<= get_pointer(o));
  }
  template <class OT>
  int compare(const OT &o) const {
    if (operator<(o)) return -1;
    else if (operator>(o)) return 1;
    else return 0;
  }
  //! Return true if the pointer is not NULL
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
  /** copy from another */
  template <class OT>
  PointerBase<O, Traits>& operator=(const OT &o){
    if (o) {
      set_pointer(static_cast<O*>(o));
    } else {
      set_pointer(NULL);
    }
    return *this;
  }
 /** copy from another */
  PointerBase<O, Traits>& operator=(const PointerBase &o){
    if (o) {
      set_pointer(static_cast<O*>(o));
    } else {
      set_pointer(NULL);
    }
    return *this;
  }
  PointerBase<O, Traits>& operator=(const long int &o){
    IMP_USAGE_CHECK(!o, "Non-null constant");
    set_pointer(NULL);
    return *this;
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
    o_= NULL;
    return ret;
  }
  void swap_with(PointerBase<O, Traits> &o) {
    std::swap(o_, o.o_);
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class T, class Traits>
inline std::ostream &operator<<(std::ostream &out,
                const std::vector<PointerBase<T, Traits> > &data) {
  out << "[";
  for (unsigned int i=0; i< data.size(); ++i) {
    if (i != 0) {
      out << ", ";
    }
    out << data[i]->get_name();
  }
  out << "]";
  return out;
}
template <class T, class Traits>
inline void swap(PointerBase<T, Traits> &a, PointerBase<T, Traits> &b) {
  a.swap_with(b);
}

template <class OT, class OTraits>
inline bool operator==(OT *o, const PointerBase<OT, OTraits> &p) {
  return p==o;
}
template <class OT, class OTraits>
inline bool operator!=(OT *o, const PointerBase<OT, OTraits> &p) {
  return p!= o;
}
template <class OT, class OTraits>
inline bool operator<(OT *o, const PointerBase<OT, OTraits> &p) {
  return p >= o;
}
template <class OT, class OTraits>
inline bool operator>(OT *o, const PointerBase<OT, OTraits> &p) {
  return p <= o;
}
template <class OT, class OTraits>
inline bool operator>=(OT *o, const PointerBase<OT, OTraits> &p) {
  return p < o;
}
template <class OT, class OTraits>
inline bool operator<=(OT *o, const PointerBase<OT, OTraits> &p) {
  return p > o;
}
#endif

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_POINTER_BASE_H */
