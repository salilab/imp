/**
 *  \file RefCountedObject.h     
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_REF_COUNTED_OBJECT_H
#define __IMP_REF_COUNTED_OBJECT_H

#include "Object.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace IMP
{

namespace internal
{

//! Common base class for ref counted objects.
/** Currently the only ref counted objects are particles.

    \note Due to weirdness in SWIG, the external objects
    are responsible for deleting the ref counted object when
    the ref count goes to zero. This will change once we have
    a real solution for Python.

   \internal
 */
class IMPDLLEXPORT RefCountedObject: public Object
{
  typedef Object P;
  typedef RefCountedObject This;
  static unsigned int live_objects_;
protected:
  RefCountedObject() {
     ++live_objects_;
  }

public:

  virtual ~RefCountedObject() {
    IMP_assert(count_==0, "Deleting object which still has references");
    --live_objects_;
  }

  void ref() {
    assert_is_valid();
    ++P::count_;
  }
  void unref() {
    assert_is_valid();
    IMP_assert(count_ != 0, "Too many unrefs on object");
    --P::count_;
  }

  static unsigned int get_number_of_live_objects() {
    return live_objects_;
  }
};



template <bool REF>
struct Ref
{
  template <class O>
  static void eval(O) {
    BOOST_STATIC_ASSERT((!boost::is_pointer<O>::value 
                         || !boost::is_base_of<RefCountedObject, O >::value));
  }
};

template <>
struct Ref<true>
{
  template <class O>
  static void eval(O* o) {
    if (o) o->ref(); 
  }
};

template <bool REF>
struct UnRef
{
  template <class O>
  static void eval(O) {
    BOOST_STATIC_ASSERT((!boost::is_pointer<O>::value 
                         || !boost::is_base_of<RefCountedObject, O >::value));
  }
};

template <>
struct UnRef<true>
{
  template <class O>
  static void eval(O *o) {
    if (o) {
      o->unref();
      if (!o->get_has_ref()) {
        delete o;
      }
    }
  }
};


//! Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O o)
{
  UnRef<(boost::is_pointer<O>::value 
         && boost::is_base_of<RefCountedObject, O >::value)>::eval(o);
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O o)
{
  Ref<(boost::is_pointer<O>::value 
       && boost::is_base_of<RefCountedObject, O >::value)>::eval(o);  
}


} // namespace internal

} // namespace IMP

#endif  /* __IMP_REF_COUNTED_OBJECT_H */
