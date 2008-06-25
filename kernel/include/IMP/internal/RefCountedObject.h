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
/** This class acts as a tag rather than providing any functionality.

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
    IMP_assert(!get_has_ref(), "Deleting object which still has references");
    --live_objects_;
  }

  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }
};



template <bool REF>
struct Ref
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCountedObject, O >::value));
    IMP_LOG(VERBOSE, "Not refing particle " << o << std::endl);
  }
};

template <>
struct Ref<true>
{
  template <class O>
  static void eval(O* o) {
    IMP_LOG(VERBOSE, "Refing particle " << o->get_index() 
            << o->get_ref_count() << std::endl);
    o->assert_is_valid();
    o->ref();
  }
};

template <bool REF>
struct UnRef
{
  template <class O>
  static void eval(O* o) {
    BOOST_STATIC_ASSERT((!boost::is_base_of<RefCountedObject, O >::value));
    IMP_LOG(VERBOSE, "Not Unrefing object " << o << std::endl);
  }
};

template <>
struct UnRef<true>
{
  template <class O>
  static void eval(O *o) {
    IMP_LOG(VERBOSE, "Unrefing particle " << o->get_index()
            << " " << o->get_ref_count() << std::endl);
    o->assert_is_valid();
    o->unref();
    if (!o->get_has_ref()) {
      delete o;
    }
  }
  };


//! Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O o)
{
  BOOST_STATIC_ASSERT(!boost::is_pointer<O>::value);
}

//! Can be called on any object and will only unref it if appropriate
template <class O>
void unref(O* o)
{
  UnRef<(boost::is_base_of<RefCountedObject, O >::value)>::eval(o);
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void ref(O* o)
{
  Ref<(boost::is_base_of<RefCountedObject, O >::value)>::eval(o);
}


//! Can be called on any object and will only unref it if appropriate
template <class O>
void disown(O* o)
{
  /*IMP_LOG(VERBOSE, "Disown called with " 
          << (boost::is_base_of<RefCountedObject, O >::value)
          << " for " << o << " " << o->get_ref_count() << std::endl);*/
  o->unref();
  if (!o->get_has_ref()) {
    delete o;
  }
}


//! Can be called on any object and will only ref it if appropriate
template <class O>
void own(O* o)
{
  /*IMP_LOG(VERBOSE, "Own called with "
          << (boost::is_base_of<RefCountedObject, O >::value)
          << " for " << o
          << " " << o->get_ref_count() << std::endl);*/
  if (boost::is_base_of<RefCountedObject, O >::value) {
    // no checks
  } else {
    IMP_check(!o->get_has_ref(), "Trying to own already owned but "
              << "non-reference-counted object: " << *o,
              ValueException);
  }
  o->ref();
}





} // namespace internal

} // namespace IMP

#endif  /* __IMP_REF_COUNTED_OBJECT_H */
