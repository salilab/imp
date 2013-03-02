/**
 *  \file ExampleRefCounted.cpp
 *  \brief An example reference counted object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/example/ExampleRefCounted.h"
#include "IMP/Pointer.h"


IMPEXAMPLE_BEGIN_NAMESPACE

ExampleRefCounted::ExampleRefCounted(const Floats &data ):
  data_(data){
}
namespace {

#ifdef __clang__
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")
#endif

void usage_example() {
  Floats data(1000, -1);

  // this would not compile
  // ExampleRefCounted rcstack;

  // create a new object and store it in a ref counted pointer
  IMP_NEW(ExampleRefCounted, rc, (data));
  // reference count is 1

  // another object with another copy of the data
  Pointer<ExampleRefCounted> rc_other= new ExampleRefCounted(data);

  // have two pointers point to the object
  Pointer<ExampleRefCounted> rc2=rc;
  // reference count is 2

  // the object is still around since rc2 points to it
  rc=static_cast<ExampleRefCounted*>(nullptr);
  // reference count is 1

  std::cout << rc2->get_data(100);
  // the object will be deleted on exit
}
}

IMPEXAMPLE_END_NAMESPACE
