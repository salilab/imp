/**
 *  \file RefCountedObject.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_REF_COUNTED_OBJECT_H
#define IMP_REF_COUNTED_OBJECT_H

#include "Object.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

IMP_BEGIN_NAMESPACE

//! Common base class for ref counted objects.
/** This class acts as a tag rather than providing any functionality.

   \internal
 */
class IMPEXPORT RefCountedObject: public Object
{
  typedef Object P;
  typedef RefCountedObject This;
  static unsigned int live_objects_;
protected:
  RefCountedObject() {
     ++live_objects_;
  }

public:
  static unsigned int get_number_of_live_objects() {
    // for debugging purposes only
    return live_objects_;
  }

protected:
  // never hold pointers to it directly
  virtual ~RefCountedObject() {
    IMP_assert(!get_has_ref(), "Deleting object which still has references");
    IMP_LOG(MEMORY, "Deleting ref counted object " << this << std::endl);
    --live_objects_;
  }

};

IMP_END_NAMESPACE

#endif  /* IMP_REF_COUNTED_OBJECT_H */
