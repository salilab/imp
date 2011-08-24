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

template <class TT>
struct RefCountedPointerTraits {
  typedef TT Type;
  static void handle_set(TT* t) {
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
  static void handle_set(TT* ) {
  }
  static void handle_unset(TT* ) {
  }
  static void check(const TT *o) {
    IMP_CHECK_OBJECT(o);
  }
};


#define IMP_POINTER_MEMBERS(templ, arg)                                 \
  templ                                                                 \
  bool operator==(arg o) const {                                        \
    return (o_== get_pointer(o));                                       \
  }                                                                     \
  templ                                                                 \
  bool operator!=(arg o) const {                                        \
    return (o_!= get_pointer(o));                                       \
  }                                                                     \
  templ                                                                 \
  bool operator<(arg o) const {                                         \
    return (o_< get_pointer(o));                                        \
  }                                                                     \
  templ                                                                 \
  bool operator>(arg o) const {                                         \
    return (o_> get_pointer(o));                                        \
  }                                                                     \
  templ                                                                 \
  bool operator>=(arg o) const {                                        \
    return (o_>= get_pointer(o));                                       \
  }                                                                     \
  templ                                                                 \
  bool operator<=(arg o) const {                                        \
    return (o_<= get_pointer(o));                                       \
  }                                                                     \
  templ                                                                 \
  int compare(arg o) const {                                            \
    if (operator<(o)) return -1;                                        \
    else if (operator>(o)) return 1;                                    \
    else return 0;                                                      \
  }                                                                     \
  templ                                                                 \
  explicit PointerBase(arg o): o_(NULL) {                               \
    if (o) {                                                            \
      set_pointer(get_pointer(o));                                      \
    }                                                                   \
  }                                                                     \
  templ                                                                 \
  PointerBase<Traits>& operator=( arg o){                               \
    if (o) {                                                            \
      set_pointer(get_pointer(o));                                      \
    } else {                                                            \
      set_pointer(NULL);                                                \
    }                                                                   \
    return *this;                                                       \
  }


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
    IMP_INTERNAL_CHECK(t != NULL, "Pointer is NULL");
    check(t);
  }
  static O* get_pointer(O*o) {return o;}
  /*static O* get_pointer(size_t t) {
    IMP_INTERNAL_CHECK(t==0, "Only can compare with NULL ints");
    return NULL;
    }*/
  template <class OT>
  static O* get_pointer(const PointerBase<OT> &o) {
    if (o) return o.get();
    else return NULL;
  }
  static O* get_pointer(const PointerBase<Traits>&o) {return o.o_;}
  static O* get_pointer(const long int) {return NULL;}
  void set_pointer(O* p) {
    if (p == o_) return;
    if (o_) Traits::handle_unset(o_);
    if (p) Traits::handle_set(p);
    check(p);
    o_=p;
  }

  struct UnusedClass{};
public:
  PointerBase(const PointerBase &o): o_(NULL) {
    set_pointer(o.o_);
  }
  //! initialize to NULL
  PointerBase(): o_(NULL) {}
  /** drop control of the object */
  ~PointerBase(){
    set_pointer(NULL);
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
  template <class O>
  PointerBase<Traits>& operator=( const O& o){
    if (o) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(NULL);
    }
    return *this;
  }
  IMP_POINTER_MEMBERS(template <class OTraits>,
                      const PointerBase<OTraits>&);
  IMP_POINTER_MEMBERS(,
                      typename Traits::Type* const);
  PointerBase<Traits>& operator=( const PointerBase & o){
    if (o) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(NULL);
    }
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
  void swap_with(PointerBase<Traits> &o) {
    std::swap(o_, o.o_);
  }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class Traits>
inline std::ostream &operator<<(std::ostream &out,
                const std::vector<PointerBase<Traits> > &data) {
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

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_POINTER_BASE_H */
