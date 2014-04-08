/**
 *  \file ExampleObject.cpp
 *  \brief An example reference counted object.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleObject.h"
#include "IMP/base/Pointer.h"

IMPEXAMPLE_BEGIN_NAMESPACE

ExampleObject::ExampleObject(const Floats &data)
    : base::Object("ExampleObject%1%"), data_(data) {}
namespace {

#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")
#endif

/** An example of how to return a new object. */
ExampleObject *create_example_object(const Floats &data) {
  IMP_NEW(ExampleObject, ret, (data));
  // one could do some work here

  // make sure it is not freed
  return ret.release();
}

void usage_example() {
  Floats data(1000, -1);

  // this would not compile
  // ExampleObject rcstack;

  // create a new object and store it in a ref counted pointer
  IMP_NEW(ExampleObject, rc, (data));
  // reference count is 1

  // another object with another copy of the data
  base::Pointer<ExampleObject> rc_other = new ExampleObject(data);

  // have two pointers point to the object
  base::Pointer<ExampleObject> rc2 = rc;
  // reference count is 2

  // the object is still around since rc2 points to it
  rc = static_cast<ExampleObject *>(nullptr);
  // reference count is 1

  std::cout << rc2->get_data(100);
  // the object will be deleted on exit
}
}

IMPEXAMPLE_END_NAMESPACE
