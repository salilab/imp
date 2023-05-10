/**
 *  \file base/internal/PointerBase.h
 *  \brief A nullptr-initialized pointer to an IMP ref-counted Object.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_POINTER_BASE_H
#define IMPKERNEL_INTERNAL_POINTER_BASE_H

#include <IMP/kernel_config.h>
#include "../check_macros.h"
#include "../warning_macros.h"
#include "../hash.h"
#include "../hash_macros.h"
#include "../Object.h"
#include <typeinfo>
#include <memory>
#include <cereal/access.hpp>
#include <type_traits>

#if defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#endif

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
template<typename O>
typename std::enable_if<std::is_default_constructible<O>::value, O*>::type
make_empty_object() {
  return new O;
}

template<typename O>
typename std::enable_if<!std::is_default_constructible<O>::value, O*>::type
make_empty_object() {
  IMP_THROW("Cannot load non-default-constructible object", TypeException);
}

class PtrWrapper {
  Object *o_;
public:
  PtrWrapper(Object *o) : o_(o) {}
  Object *get_object() { return o_; }
};

} // namespace
#endif

template <class TT>
struct RefCountedPointerTraits {
  typedef TT Type;
  static void handle_set(TT* t) {
    IMP_CHECK_OBJECT_IF_NOT_nullptr(t);
    if (t) t->ref();
  }
  static void handle_unset(TT* t) {
    if (t) t->unref();
  }
  static void check(const TT* o) { IMP_CHECK_OBJECT(o); }
  static void release(TT* o) {
    if (o) o->release();
  }
};
template <class TT>
// note: PointerMember replaces the old OwnerPointer
struct PointerMemberTraits : public RefCountedPointerTraits<TT> {
  typedef TT Type;
  static void handle_set(TT* t) {
    t->set_was_used(true);
    RefCountedPointerTraits<TT>::handle_set(t);
  }
};
template <class TT>
struct WeakPointerTraits {
  typedef TT Type;
  static void handle_set(TT*) {}
  static void handle_unset(TT*) {}
  static void check(const TT*) {}
  static void release(TT*) {}
};

template <class TT>
struct CheckedWeakPointerTraits {
  typedef TT Type;
  static void handle_set(TT* o) {
    IMP_CHECK_VARIABLE(o);
    IMP_CHECK_OBJECT_IF_NOT_nullptr(o);
  }
  static void handle_unset(TT*) {}
  static void check(const TT* o) { IMP_CHECK_OBJECT(o); }
  static void release(TT*) {}
};

template <class O, class OO, class Enabled = void>
struct GetPointer {
  static O* get_pointer(const OO& o) { return o; }
  static const O* get_const_pointer(const OO& o) { return o; }
};
#if(defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)) && \
    !defined(nullptr)
template <class O, class OO>
struct GetPointer<O, OO,
                  typename boost::enable_if<boost::mpl::and_<
                      boost::mpl::not_<std::is_integral<OO>::value>,
                      boost::mpl::not_<std::is_pointer<OO>::value> > >::type> {
  static O* get_pointer(const OO& o) { return o; }
  static const O* get_const_pointer(const OO& o) { return o; }
};
template <class O, class OO>
struct GetPointer<O, OO*,
                typename boost::enable_if<std::is_pointer<OO*>::value>::type> {
  static O* get_pointer(OO* o) { return o; }
  static const O* get_const_pointer(const OO* o) { return o; }
};

template <class O, class OO>
struct GetPointer<O, OO,
                typename boost::enable_if<std::is_integral<OO>::value>::type> {
  static O* get_pointer(const OO& o) {
    IMP_INTERNAL_CHECK_VARIABLE(o)
    IMP_INTERNAL_CHECK(o == 0, "Non-zero pointer constant found.");
    return static_cast<O*>(nullptr);
  }
  static const O* get_const_pointer(const OO& o) {
    IMP_INTERNAL_CHECK_VARIABLE(o);
    IMP_INTERNAL_CHECK(o == 0, "Non-zero pointer constant found.");
    return static_cast<O*>(nullptr);
  }
};
template <class O>
struct GetPointer<O, std::nullptr_t> {
  static O* get_pointer(const std::nullptr_t&) {
    return static_cast<O*>(nullptr);
  }
  static const O* get_const_pointer(const std::nullptr_t&) {
    return static_cast<O*>(nullptr);
  }
};

#endif

template <class Traits>
class PointerBase {
 public:
  typedef typename Traits::Type O;

 private:
  O* o_;
  static void check(const O* o) {
    if (o) {
      Traits::check(o);
    }
  }
  static void check_non_null(const O* t) {
    IMP_INTERNAL_CHECK(t, "Pointer is nullptr");
    check(t);
  }
  // static O* get_pointer(O*o) {return o;}
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
    // so the pointer is clear if inspected during cleanup
    O* t = o_;
    o_ = p;
    if (t) Traits::handle_unset(t);
  }

  struct UnusedClass {};

  friend class cereal::access;

  static void null_deleter(Object *) {}

  void serialize(cereal::BinaryOutputArchive &ar) {
    O* rawptr = o_;
    if (rawptr == nullptr) {
      char ptr_type = 0; // null pointer
      ar(ptr_type);
    } else {
      // cereal wants a shared_ptr, but we manage the storage for Object
      // ourselves, so provide a null deleter. This will potentially make
      // multiple shared_ptr objects pointing to the same Object*, but that's
      // OK here because cereal only uses the underlying pointer anyway.
      std::shared_ptr<Object> shared_rawptr(rawptr, null_deleter);
      uint32_t id = ar.registerSharedPointer(shared_rawptr);
      if (typeid(*rawptr) == typeid(O)) {
        char ptr_type = 1; // non-polymorphic pointer
        ar(ptr_type);
        ar(id);
        // only serialize if this is the first time we've seen this ID
        if (id & cereal::detail::msb_32bit) {
          ar(*rawptr);
        }
      } else {
        char ptr_type = 2; // polymorphic pointer
        ar(ptr_type);
        ar(id);
        // only serialize if this is the first time we've seen this ID
        if (id & cereal::detail::msb_32bit) {
          rawptr->poly_serialize(ar);
        }
      }
    }
  }

  void serialize(cereal::BinaryInputArchive &ar) {
    char ptr_type;
    uint32_t id;
    ar(ptr_type);
    if (ptr_type == 0) { // null pointer
      set_pointer(nullptr);
    } else {
      ar(id);
      if (ptr_type == 1) { // non-polymorphic pointer
        // only deserialize if this is the first time we've seen this ID
        if (id & cereal::detail::msb_32bit) {
          std::unique_ptr<O> ptr(make_empty_object<O>());
          ar(*ptr);
          auto ptr_wrapper = std::make_shared<PtrWrapper>(ptr.get());
          set_pointer(ptr.release());
          ar.registerSharedPointer(id, ptr_wrapper);
        } else {
          set_pointer_from_id(id, ar);
        }
      } else { // polymorphic pointer
        // only deserialize if this is the first time we've seen this ID
        if (id & cereal::detail::msb_32bit) {
          O* rawptr = dynamic_cast<O*>(Object::poly_unserialize(ar));
          IMP_INTERNAL_CHECK(rawptr != nullptr, "Wrong type returned");
          set_pointer(rawptr);
          auto ptr_wrapper = std::make_shared<PtrWrapper>(rawptr);
          ar.registerSharedPointer(id, ptr_wrapper);
        } else {
          set_pointer_from_id(id, ar);
        }
      }
    }
  }

  void set_pointer_from_id(uint32_t id, cereal::BinaryInputArchive &ar) {
    auto ptr_wrapper = std::static_pointer_cast<PtrWrapper>(
                                              ar.getSharedPointer(id));
    O* rawptr = dynamic_cast<O*>(ptr_wrapper->get_object());
    IMP_INTERNAL_CHECK(rawptr != nullptr, "Wrong type returned");
    set_pointer(rawptr);
  }

 public:
  //! initialize to nullptr
  PointerBase() : o_(nullptr) {}
  /** drop control of the object */
  ~PointerBase() {
    // to make sure it is cleared
    set_pointer(nullptr);
  }
  //! Return true if the pointer is not nullptr
  bool operator!() const { return !o_; }
  //! convert to the raw pointer
  operator O*() const { return o_; }
  IMP_HASHABLE_INLINE(PointerBase, return boost::hash_value(o_););
  const O& operator*() const {
    check_non_null(o_);
    return *o_;
  }
  O& operator*() {
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
    return (o_ == get_const_pointer(o));
  }
  template <class OO>
  bool operator!=(const OO& o) const {
    return (o_ != get_const_pointer(o));
  }
  template <class OO>
  bool operator<(const OO& o) const {
    return (o_ < get_const_pointer(o));
  }
  template <class OO>
  bool operator>(const OO& o) const {
    return (o_ > get_const_pointer(o));
  }
  template <class OO>
  bool operator>=(const OO& o) const {
    return (o_ >= get_const_pointer(o));
  }
  template <class OO>
  bool operator<=(const OO& o) const {
    return (o_ <= get_const_pointer(o));
  }
  template <class OO>
  int compare(const OO& o) const {
    if (operator<(o))
      return -1;
    else if (operator>(o))
      return 1;
    else
      return 0;
  }
  template <class OO>
  explicit PointerBase(const OO& o)
      : o_(nullptr) {
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    }
  }

  template <class OT>
  PointerBase<Traits>& operator=(const PointerBase<OT>& o) {
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(nullptr);
    }
    return *this;
  }
  template <class OT>
  PointerBase<Traits>& operator=(OT* o) {
    if (get_pointer(o)) {
      set_pointer(get_pointer(o));
    } else {
      set_pointer(nullptr);
    }
    return *this;
  }
  PointerBase<Traits>& operator=(std::nullptr_t) {
    set_pointer(nullptr);
    return *this;
  }
  PointerBase<Traits>& operator=(const PointerBase<Traits>& o) {
    set_pointer(o.o_);
    return *this;
  }
  /*IMP_POINTER_MEMBERS(template <class OO>,
    OO*);*/
  PointerBase(const PointerBase& o) : o_(nullptr) { set_pointer(o.o_); }

  //! Relinquish control of the pointer
  /** This must be the only pointer pointing to the object. Its
      reference count will be 0 after the function is called, but
      the object will not be destroyed. Use this to safely return
      objects allocated within functions.
  */
  O* release() {
    Traits::release(o_);
    O* ret = o_;
    o_ = nullptr;
    return ret;
  }
  void swap_with(PointerBase<Traits>& o) { std::swap(o_, o.o_); }
};

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class Traits>
inline void swap(PointerBase<Traits>& a, PointerBase<Traits>& b) {
  a.swap_with(b);
}

// Reversed operators are not needed in C++20
#ifndef IMP_COMPILER_HAS_THREE_WAY
template <class OT, class OTraits>
inline bool operator==(OT* o, const PointerBase<OTraits>& p) {
  return p == o;
}
template <class OT, class OTraits>
inline bool operator!=(OT* o, const PointerBase<OTraits>& p) {
  return p != o;
}
template <class OT, class OTraits>
inline bool operator<(OT* o, const PointerBase<OTraits>& p) {
  return p > o;
}
template <class OT, class OTraits>
inline bool operator>(OT* o, const PointerBase<OTraits>& p) {
  return p < o;
}
template <class OT, class OTraits>
inline bool operator>=(OT* o, const PointerBase<OTraits>& p) {
  return p <= o;
}
template <class OT, class OTraits>
inline bool operator<=(OT* o, const PointerBase<OTraits>& p) {
  return p >= o;
}
#endif
#endif

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_POINTER_BASE_H */
